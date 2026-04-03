const BUILD_VERSION = "Web-v6.4";
const CACHE_NAME = BUILD_VERSION;
const APP_SHELL = [
  "./",
  "./index.html",
  "./guide.html",
  "./session.html",
  "./styles.css",
  "./app.js",
  "./session.js",
  "./manifest.json",
  "./icons/icon-192.png",
  "./icons/icon-512.png",
  "./404.html",
  "./icons/ota-download-green.png"
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
    cache.put(request, fresh.clone());
    return fresh;
  }).catch(() => cached);
  return cached || fetchPromise;
}

self.addEventListener("fetch", (event) => {
  const req = event.request;
  if (req.method !== "GET") return;

  const url = new URL(req.url);
  const sameOrigin = url.origin === self.location.origin;

  if (req.mode === "navigate") {
    event.respondWith(networkFirst(req, "./index.html"));
    return;
  }

  if (sameOrigin && ["style", "script", "manifest"].includes(req.destination)) {
    event.respondWith(networkFirst(req, "./index.html"));
    return;
  }

  if (sameOrigin && ["image", "font"].includes(req.destination)) {
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
