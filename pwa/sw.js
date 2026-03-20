const BUILD_VERSION = "TSPweb-v2.1.0";
const CACHE_NAME = BUILD_VERSION;
const APP_SHELL = [
  "./",
  "./index.html",
  "./session.html",
  "./guide.html",
  "./styles.css",
  "./app.js",
  "./session.js",
  "./manifest.json",
  "./icons/icon-192.png",
  "./icons/icon-512.png",
  "./404.html"
];

self.addEventListener("message", (event) => {
  if (event.data && event.data.type === "SKIP_WAITING") self.skipWaiting();
});

self.addEventListener("install", (event) => {
  self.skipWaiting();
  event.waitUntil(caches.open(CACHE_NAME).then((cache) => cache.addAll(APP_SHELL)));
});

self.addEventListener("activate", (event) => {
  event.waitUntil((async () => {
    const keys = await caches.keys();
    await Promise.all(keys.map((k) => (k !== CACHE_NAME ? caches.delete(k) : null)));
    await self.clients.claim();
  })());
});

async function networkFirst(request, fallbackUrl = "./index.html") {
  try {
    const fresh = await fetch(request, { cache: "no-store" });
    if (!fresh.ok && fresh.type !== "opaque") throw new Error(`HTTP ${fresh.status}`);
    const cache = await caches.open(CACHE_NAME);
    cache.put(request, fresh.clone());
    return fresh;
  } catch {
    const cached = await caches.match(request, { ignoreSearch: true });
    return cached || caches.match(fallbackUrl, { ignoreSearch: true }) || caches.match("./404.html");
  }
}

async function staleWhileRevalidate(request) {
  const cache = await caches.open(CACHE_NAME);
  const cached = await cache.match(request, { ignoreSearch: true });
  const fetchPromise = fetch(request).then((fresh) => {
    if (fresh && (fresh.ok || fresh.type === "opaque")) cache.put(request, fresh.clone());
    return fresh;
  }).catch(() => cached);
  return cached || fetchPromise;
}

const SCOPE_PATH = (() => {
  const p = new URL(self.registration.scope).pathname;
  if (p === "/") return "";
  return p.endsWith("/") ? p.slice(0, -1) : p;
})();

function navigationFallbackFor(url) {
  const rawPath = url.pathname || "/";
  const normalizedPath = rawPath !== "/" && rawPath.endsWith("/") ? rawPath.slice(0, -1) : rawPath;
  const rootPath = SCOPE_PATH || "";

  if (normalizedPath === "/" || normalizedPath === rootPath || normalizedPath === `${rootPath}/index.html`) return "./index.html";
  if (normalizedPath === `${rootPath}/session.html`) return "./session.html";
  if (normalizedPath === `${rootPath}/guide.html`) return "./guide.html";
  if (normalizedPath === `${rootPath}/404.html`) return "./404.html";
  return "./404.html";
}

self.addEventListener("fetch", (event) => {
  const req = event.request;
  if (req.method !== "GET") return;

  const url = new URL(req.url);
  const sameOrigin = url.origin === self.location.origin;

  if (req.mode === "navigate") {
    event.respondWith(networkFirst(req, navigationFallbackFor(url)));
    return;
  }

  if (sameOrigin && ["style", "script", "image", "font"].includes(req.destination)) {
    event.respondWith(staleWhileRevalidate(req));
    return;
  }

  if (sameOrigin) {
    event.respondWith(networkFirst(req, "./404.html"));
    return;
  }

  event.respondWith(fetch(req).catch(() => caches.match(req)));
});

self.addEventListener("notificationclick", (event) => {
  event.notification.close();
  event.waitUntil((async () => {
    const all = await clients.matchAll({ type: "window", includeUncontrolled: true });
    for (const c of all) {
      if ("focus" in c) return c.focus();
    }
    if (clients.openWindow) return clients.openWindow("./index.html");
  })());
});
