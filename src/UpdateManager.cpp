#include "UpdateManager.h"
#include "FirebaseNetwork.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <esp_attr.h>
#include <esp_app_format.h>
#include <esp_http_client.h>
#include <esp_heap_caps.h>
#include <memory>
#include <stdlib.h>

RTC_DATA_ATTR static uint32_t s_magic = 0;
RTC_DATA_ATTR static uint32_t s_lastAppAddr = 0;
RTC_DATA_ATTR static uint8_t  s_crashBoots = 0;
RTC_DATA_ATTR static uint8_t  s_safeMode = 0;

static constexpr uint32_t MAGIC = 0xC0FFEE42;
static const uint32_t OTA_HTTP_TIMEOUT_MS   = 60000;
static const uint32_t OTA_STREAM_IDLE_MS    = 30000;
static const uint32_t OTA_RESTART_DELAY_MS  = 1200;
static const size_t   OTA_MIN_BIN_BYTES     = 128 * 1024;

static bool isCrashReset(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
      return true;
    default:
      return false;
  }
}

static bool getPendingVerifyState_() {
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;

  esp_ota_img_states_t st;
  if (esp_ota_get_state_partition(running, &st) != ESP_OK) return false;
  return (st == ESP_OTA_IMG_PENDING_VERIFY);
}

String UpdateManager::normalizeFirmwareUrl_(String url) {
  url.trim();
  url.replace("?raw=1", "");
  if (!url.startsWith("https://github.com/")) return url;

  const String prefix = "https://github.com/";
  String tail = url.substring(prefix.length());
  const int blobAt = tail.indexOf("/blob/");
  if (blobAt < 0) return url;

  const String head = tail.substring(0, blobAt);
  const String rest = tail.substring(blobAt + 6);

  String out = "https://raw.githubusercontent.com/";
  out += head;
  out += "/";
  out += rest;
  return out;
}

static inline String otaErr_(const char* msg, int code = 0) {
  String s = msg ? String(msg) : String("OTA_ERROR");
  if (code != 0) {
    s += " (";
    s += String(code);
    s += ")";
  }
  return s;
}

bool UpdateManager::findRollbackPartition_(const esp_partition_t*& out) const {
  out = nullptr;

  const esp_partition_t* running = esp_ota_get_running_partition();
  if (!running) return false;

  const esp_partition_t* candidate = esp_ota_get_next_update_partition(running);
  if (!candidate || candidate == running) return false;

  esp_app_desc_t desc = {};
  if (esp_ota_get_partition_description(candidate, &desc) != ESP_OK) return false;

  uint8_t magic = 0;
  if (esp_partition_read(candidate, 0, &magic, sizeof(magic)) != ESP_OK) return false;
  if (magic != ESP_IMAGE_HEADER_MAGIC) return false;

  out = candidate;
  return true;
}

bool UpdateManager::rollbackToPrevious() {
  const esp_partition_t* part = nullptr;
  if (!findRollbackPartition_(part)) return false;

  const esp_err_t e = esp_ota_set_boot_partition(part);
  if (e != ESP_OK) return false;

  delay(120);
  ESP.restart();
  return true;
}

bool UpdateManager::confirmNow() {
  if (!_pendingVerify) return true;

  const esp_err_t e = esp_ota_mark_app_valid_cancel_rollback();
  if (e == ESP_OK) {
    _pendingVerify = false;
    _crashBoots = 0;
    s_crashBoots = 0;
    return true;
  }
  return false;
}

void UpdateManager::bootGuardBegin_(uint32_t stableWindowMs, uint8_t maxCrashBoots) {
  _stableWindowMs = (stableWindowMs < 5000) ? 5000 : stableWindowMs;
  _maxCrashBoots  = (maxCrashBoots < 1) ? 1 : maxCrashBoots;
  _bootMs0 = millis();

  if (s_magic != MAGIC) {
    s_magic = MAGIC;
    s_lastAppAddr = 0;
    s_crashBoots = 0;
    s_safeMode = 0;
  }

  const esp_partition_t* running = esp_ota_get_running_partition();
  const uint32_t runAddr = running ? running->address : 0;

  if (runAddr != 0 && s_lastAppAddr != 0 && runAddr != s_lastAppAddr) {
    s_crashBoots = 0;
    s_safeMode = 0;
  }
  if (runAddr != 0) s_lastAppAddr = runAddr;

  const bool crash = isCrashReset(esp_reset_reason());
  if (crash) {
    if (s_crashBoots < 255) s_crashBoots++;
  } else {
    s_crashBoots = 0;
  }

  _crashBoots = s_crashBoots;
  _pendingVerify = getPendingVerifyState_();

  if (_pendingVerify && crash && s_crashBoots >= _maxCrashBoots) {
    (void)esp_ota_mark_app_invalid_rollback_and_reboot();
  }

  if (crash && s_crashBoots >= _maxCrashBoots) s_safeMode = 1;
  _safeMode = (s_safeMode != 0);
}

void UpdateManager::bootGuardLoop_() {
  if (_safeMode) return;

  const uint32_t up = millis() - _bootMs0;
  if (up < _stableWindowMs) return;

  s_crashBoots = 0;
  _crashBoots = 0;

  if (_pendingVerify) (void)confirmNow();
}

void UpdateManager::begin(const char* currentVersion, FirebaseNetwork* cloud, uint32_t stableWindowMs, uint8_t maxCrashBoots) {
  _currentVersion = currentVersion ? currentVersion : "TSP-v0.0.0";
  _cloud = cloud;
  _checkNow = true;
  _lastCheckMs = 0;
  _lastError = "";
  bootGuardBegin_(stableWindowMs, maxCrashBoots);
}

void UpdateManager::setPaths(const char* desiredVersionPath, const char* firmwareUrlPath) {
  if (desiredVersionPath && strlen(desiredVersionPath)) _pathDesired = desiredVersionPath;
  if (firmwareUrlPath && strlen(firmwareUrlPath)) _pathUrl = firmwareUrlPath;
}

void UpdateManager::setCheckInterval(uint32_t ms) {
  _intervalMs = ms;
}

void UpdateManager::requestCheckNow() {
  _checkNow = true;
}

void UpdateManager::setInsecureTLS(bool en) {
  _insecureTLS = en;
}

void UpdateManager::loop() {
  bootGuardLoop_();

  // Confirm as early as possible after a successful boot.
  if (!confirmNow()) return;

  if (WiFi.status() != WL_CONNECTED || !_cloud || !_cloud->isReady()) return;

  const uint32_t now = millis();
  if (!_checkNow && (uint32_t)(now - _lastCheckMs) < _intervalMs) return;

  _checkNow = false;
  _lastCheckMs = now;
  _lastError = "";

  String desiredVer, fwUrl;
  if (!fetchOtaTargets(desiredVer, fwUrl)) {
    _lastError = otaErr_("OTA_TARGET_FETCH_FAILED");
    return;
  }

  desiredVer.trim();
  fwUrl.trim();

  if (!desiredVer.length()) {
    _lastError = otaErr_("OTA_EMPTY_VERSION");
    return;
  }
  if (!fwUrl.length()) {
    _lastError = otaErr_("OTA_EMPTY_URL");
    return;
  }
  if (desiredVer.equals(_currentVersion)) {
    _lastError = otaErr_("OTA_SKIP_SAME_VERSION");
    return;
  }

  if (_cb) _cb(OtaEvent::START, 0);

  if (performUpdateFromUrl(fwUrl)) {
    if (_cb) _cb(OtaEvent::SUCCESS, 100);
    delay(OTA_RESTART_DELAY_MS);
    ESP.restart();
  } else {
    if (_cb) _cb(OtaEvent::FAIL, 0);
  }
}

bool UpdateManager::fetchOtaTargets(String& desiredVersion, String& firmwareUrl) {
  if (!_cloud) return false;
  if (!_cloud->getString(_pathDesired, desiredVersion)) return false;
  if (!_cloud->getString(_pathUrl, firmwareUrl)) return false;
  return true;
}


bool UpdateManager::performUpdateFromUrl(const String& rawUrl) {
  struct ParsedUrl {
    bool https = true;
    String host;
    uint16_t port = 443;
    String path = "/";
  };

  auto parseUrl = [&](const String& in, ParsedUrl& out) -> bool {
    String url = in;
    url.trim();
    const bool https = url.startsWith("https://");
    const bool http  = url.startsWith("http://");
    if (!https && !http) return false;

    const int schemeLen = https ? 8 : 7;
    String rest = url.substring(schemeLen);
    int slash = rest.indexOf('/');
    String hostPort = (slash >= 0) ? rest.substring(0, slash) : rest;
    String path = (slash >= 0) ? rest.substring(slash) : String("/");
    hostPort.trim();
    path.trim();
    if (!hostPort.length()) return false;
    if (!path.length()) path = "/";

    int colon = hostPort.lastIndexOf(':');
    String host = hostPort;
    uint16_t port = https ? 443 : 80;
    if (colon > 0 && colon < (int)hostPort.length() - 1) {
      const String portStr = hostPort.substring(colon + 1);
      const int parsed = portStr.toInt();
      if (parsed <= 0 || parsed > 65535) return false;
      port = (uint16_t)parsed;
      host = hostPort.substring(0, colon);
    }

    host.trim();
    if (!host.length()) return false;
    out.https = https;
    out.host = host;
    out.port = port;
    out.path = path;
    return true;
  };

  auto followRedirect = [&](const String& currentUrl, const String& location, String& nextUrl) -> bool {
    String loc = location;
    loc.trim();
    if (!loc.length()) return false;
    if (loc.startsWith("http://") || loc.startsWith("https://")) {
      nextUrl = loc;
      return true;
    }

    ParsedUrl cur = {};
    if (!parseUrl(currentUrl, cur)) return false;

    if (loc.startsWith("//")) {
      nextUrl = String(cur.https ? "https:" : "http:") + loc;
      return true;
    }

    if (loc.startsWith("/")) {
      nextUrl = String(cur.https ? "https://" : "http://") + cur.host;
      if ((cur.https && cur.port != 443) || (!cur.https && cur.port != 80)) {
        nextUrl += ":" + String(cur.port);
      }
      nextUrl += loc;
      return true;
    }

    String basePath = cur.path;
    int q = basePath.indexOf('?');
    if (q >= 0) basePath = basePath.substring(0, q);
    int lastSlash = basePath.lastIndexOf('/');
    if (lastSlash < 0) basePath = "/";
    else basePath = basePath.substring(0, lastSlash + 1);

    nextUrl = String(cur.https ? "https://" : "http://") + cur.host;
    if ((cur.https && cur.port != 443) || (!cur.https && cur.port != 80)) {
      nextUrl += ":" + String(cur.port);
    }
    nextUrl += basePath + loc;
    return true;
  };

  auto readLine = [](Client& c, String& line, uint32_t timeoutMs) -> bool {
    line = "";
    const uint32_t t0 = millis();
    while ((millis() - t0) < timeoutMs) {
      while (c.available()) {
        char ch = (char)c.read();
        if (ch == '\r') continue;
        if (ch == '\n') return true;
        line += ch;
      }
      delay(1);
    }
    return false;
  };

  auto decodeChunkedWrite = [&](Client& stream, size_t& written, int total, uint32_t& lastDataMs, int& lastPct) -> bool {
    uint8_t buf[2048];
    while (true) {
      String sizeLine;
      if (!readLine(stream, sizeLine, OTA_STREAM_IDLE_MS)) {
        _lastError = otaErr_("OTA_CHUNK_SIZE_TIMEOUT");
        return false;
      }
      const int semi = sizeLine.indexOf(';');
      if (semi >= 0) sizeLine = sizeLine.substring(0, semi);
      sizeLine.trim();
      if (!sizeLine.length()) continue;
      const size_t chunkSize = (size_t)strtoul(sizeLine.c_str(), nullptr, 16);
      if (chunkSize == 0) {
        String trailer;
        do {
          if (!readLine(stream, trailer, 5000UL)) break;
        } while (trailer.length() > 0);
        return true;
      }

      size_t remain = chunkSize;
      while (remain > 0) {
        if (!stream.connected() && stream.available() == 0) {
          _lastError = otaErr_("OTA_CHUNK_EOF");
          return false;
        }
        const size_t avail = stream.available();
        if (avail == 0) {
          if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) {
            _lastError = otaErr_("OTA_STREAM_IDLE_TIMEOUT");
            return false;
          }
          delay(1);
          continue;
        }
        const size_t want = (avail < sizeof(buf)) ? avail : sizeof(buf);
        const size_t take = (want < remain) ? want : remain;
        const int r = stream.readBytes(buf, take);
        if (r <= 0) {
          if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) {
            _lastError = otaErr_("OTA_STREAM_IDLE_TIMEOUT");
            return false;
          }
          delay(1);
          continue;
        }
        lastDataMs = millis();
        const size_t w = Update.write(buf, (size_t)r);
        if (w != (size_t)r) {
          _lastError = otaErr_("OTA_WRITE_FAILED", Update.getError());
          return false;
        }
        written += w;
        remain -= (size_t)r;
        if (total > 0 && _cb) {
          const int pct = (int)((100.0f * (float)written) / (float)total);
          if (pct != lastPct) {
            lastPct = pct;
            _cb(OtaEvent::PROGRESS, pct);
          }
        }
      }
      String crlf;
      if (!readLine(stream, crlf, 5000UL)) {
        _lastError = otaErr_("OTA_CHUNK_TERM_TIMEOUT");
        return false;
      }
    }
  };

  String baseUrl = normalizeFirmwareUrl_(rawUrl);
  ParsedUrl parsed = {};
  if (!parseUrl(baseUrl, parsed)) {
    _lastError = otaErr_("OTA_BAD_URL");
    return false;
  }

  const esp_partition_t* target = esp_ota_get_next_update_partition(nullptr);
  if (!target) {
    _lastError = otaErr_("OTA_NO_TARGET_PARTITION");
    return false;
  }

  auto doAttempt = [&](const String& url) -> bool {
    String currentUrl = url;
    for (int redirectCount = 0; redirectCount < 4; ++redirectCount) {
      ParsedUrl u = {};
      if (!parseUrl(currentUrl, u)) {
        _lastError = otaErr_("OTA_BAD_URL");
        return false;
      }

      if (_cloud) {
        _cloud->stopAllClients();
        delay(150);
      }

      IPAddress resolvedIp;
      if (!WiFi.hostByName(u.host.c_str(), resolvedIp)) {
        _lastError = String("OTA_DNS_FAIL ") + u.host;
        return false;
      }

      const uint32_t heapBeforeClient = ESP.getFreeHeap();
      const uint32_t largestBeforeClient = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

      std::unique_ptr<WiFiClient> plain;
      std::unique_ptr<WiFiClientSecure> secure;
      Client* client = nullptr;
      if (u.https) {
        secure.reset(new WiFiClientSecure());
        if (!secure) {
          _lastError = String("OTA_CLIENT_ALLOC_FAIL heap=") + String(heapBeforeClient) +
                       String(" largest=") + String(largestBeforeClient);
          return false;
        }
        secure->setTimeout(OTA_HTTP_TIMEOUT_MS / 1000);
        secure->setHandshakeTimeout(20);
        if (_insecureTLS) secure->setInsecure();
        client = secure.get();
      } else {
        plain.reset(new WiFiClient());
        if (!plain) {
          _lastError = String("OTA_CLIENT_ALLOC_FAIL heap=") + String(heapBeforeClient) +
                       String(" largest=") + String(largestBeforeClient);
          return false;
        }
        plain->setTimeout(OTA_HTTP_TIMEOUT_MS / 1000);
        client = plain.get();
      }

      const uint32_t heapBeforeConnect = ESP.getFreeHeap();
      const uint32_t largestBeforeConnect = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
      if (!client->connect(u.host.c_str(), u.port)) {
        String detail = String("OTA_CONNECT_FAIL ") + u.host + ":" + String(u.port);
        if (u.https && secure) {
          char errbuf[128] = {0};
          secure->lastError(errbuf, sizeof(errbuf));
          if (errbuf[0]) {
            detail += "|";
            detail += errbuf;
          }
        }
        detail += String("|heap=") + String(heapBeforeConnect);
        detail += String("|largest=") + String(largestBeforeConnect);
        _lastError = detail;
        return false;
      }

      String req;
      req.reserve(256 + u.path.length() + u.host.length());
      req += "GET "; req += u.path; req += " HTTP/1.1\r\n";
      req += "Host: "; req += u.host; req += "\r\n";
      req += "User-Agent: TinyML-SmartPlug-OTA/2.0\r\n";
      req += "Accept: */*\r\n";
      req += "Accept-Encoding: identity\r\n";
      req += "Connection: close\r\n";
      req += "Cache-Control: no-cache\r\n\r\n";
      client->print(req);

      String statusLine;
      if (!readLine(*client, statusLine, 15000UL)) {
        client->stop();
        _lastError = otaErr_("OTA_NO_STATUS_LINE");
        return false;
      }
      statusLine.trim();
      if (!statusLine.startsWith("HTTP/1.")) {
        client->stop();
        _lastError = String("OTA_BAD_STATUS ") + statusLine;
        return false;
      }

      int sp1 = statusLine.indexOf(' ');
      int sp2 = (sp1 >= 0) ? statusLine.indexOf(' ', sp1 + 1) : -1;
      const String codeStr = (sp1 >= 0) ? statusLine.substring(sp1 + 1, (sp2 > sp1 ? sp2 : statusLine.length())) : String("");
      const int code = codeStr.toInt();

      int total = -1;
      bool chunked = false;
      String location = "";
      while (true) {
        String line;
        if (!readLine(*client, line, 15000UL)) {
          client->stop();
          _lastError = otaErr_("OTA_HEADER_TIMEOUT");
          return false;
        }
        if (line.length() == 0) break;

        const int colon = line.indexOf(':');
        if (colon <= 0) continue;
        String key = line.substring(0, colon);
        String val = line.substring(colon + 1);
        key.trim(); val.trim();
        key.toLowerCase();
        val.trim();

        if (key == "content-length") total = val.toInt();
        else if (key == "transfer-encoding") {
          String lv = val; lv.toLowerCase();
          if (lv.indexOf("chunked") >= 0) chunked = true;
        } else if (key == "location") {
          location = val;
        }
      }

      if (code >= 300 && code < 400) {
        client->stop();
        String nextUrl;
        if (!followRedirect(currentUrl, location, nextUrl)) {
          _lastError = String("OTA_REDIRECT_FAIL ") + String(code);
          return false;
        }
        currentUrl = nextUrl;
        continue;
      }

      if (code != 200) {
        client->stop();
        _lastError = otaErr_("OTA_HTTP_STATUS", code);
        return false;
      }

      if (total > 0 && (size_t)total > target->size) {
        client->stop();
        _lastError = otaErr_("OTA_IMAGE_TOO_LARGE", total);
        return false;
      }
      if (total > 0 && (size_t)total < OTA_MIN_BIN_BYTES) {
        client->stop();
        _lastError = otaErr_("OTA_FILE_TOO_SMALL", total);
        return false;
      }

      const uint32_t firstWaitMs = millis();
      while (client->available() == 0 && client->connected() && (millis() - firstWaitMs) < 5000UL) {
        delay(1);
      }
      if (client->available() == 0) {
        client->stop();
        _lastError = otaErr_("OTA_NO_BODY");
        return false;
      }

      const int firstByte = client->peek();
      if (firstByte != ESP_IMAGE_HEADER_MAGIC) {
        client->stop();
        _lastError = otaErr_("OTA_BAD_IMAGE_HEADER", firstByte);
        return false;
      }

      Update.abort();
      if (!Update.begin(total > 0 ? (size_t)total : UPDATE_SIZE_UNKNOWN, U_FLASH)) {
        client->stop();
        _lastError = otaErr_("OTA_BEGIN_FAILED", Update.getError());
        return false;
      }

      size_t written = 0;
      uint32_t lastDataMs = millis();
      int lastPct = -1;
      bool ok = true;

      if (chunked) {
        ok = decodeChunkedWrite(*client, written, total, lastDataMs, lastPct);
      } else {
        uint8_t buf[2048];
        while (true) {
          const size_t avail = client->available();
          if (avail > 0) {
            const size_t want = (avail > sizeof(buf)) ? sizeof(buf) : avail;
            const int r = client->readBytes(buf, want);
            if (r > 0) {
              lastDataMs = millis();
              const size_t w = Update.write(buf, (size_t)r);
              if (w != (size_t)r) {
                _lastError = otaErr_("OTA_WRITE_FAILED", Update.getError());
                ok = false;
                break;
              }
              written += w;
              if (total > 0 && _cb) {
                const int pct = (int)((100.0f * (float)written) / (float)total);
                if (pct != lastPct) {
                  lastPct = pct;
                  _cb(OtaEvent::PROGRESS, pct);
                }
              }
              continue;
            }
          }

          const bool doneByLength = (total > 0) && (written >= (size_t)total);
          const bool doneByEof = (total <= 0) && !client->connected() && (client->available() == 0);
          if (doneByLength || doneByEof) break;
          if ((millis() - lastDataMs) > OTA_STREAM_IDLE_MS) {
            _lastError = otaErr_("OTA_STREAM_IDLE_TIMEOUT");
            ok = false;
            break;
          }
          delay(1);
        }
      }

      client->stop();

      if (!ok) {
        Update.abort();
        return false;
      }

      if (total > 0 && written != (size_t)total) {
        Update.abort();
        _lastError = otaErr_("OTA_INCOMPLETE_IMAGE", (int)written);
        return false;
      }

      if (!Update.end(true) || !Update.isFinished()) {
        _lastError = otaErr_("OTA_END_FAILED", Update.getError());
        return false;
      }

      _lastError = "";
      return true;
    }

    _lastError = otaErr_("OTA_TOO_MANY_REDIRECTS");
    return false;
  };

  String lastErr = "";
  for (int attempt = 0; attempt < 3; ++attempt) {
    String tryUrl = baseUrl;
    const String sep = (tryUrl.indexOf('?') >= 0) ? "&" : "?";
    tryUrl += sep;
    tryUrl += "cb=";
    tryUrl += String((uint32_t)millis());
    tryUrl += "_";
    tryUrl += String(attempt + 1);

    if (doAttempt(tryUrl)) return true;
    lastErr = _lastError;
    delay(1200);
  }

  _lastError = lastErr.length() ? lastErr : otaErr_("OTA_ALL_ATTEMPTS_FAILED");
  return false;
}
