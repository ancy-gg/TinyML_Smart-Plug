const BUILD_VERSION = "Web-v8.6.0";
const CACHE_NAME = `tsp-shell-${BUILD_VERSION}`;
const APP_SHELL = [
  "./",
  "./index.html",
  "./guide.html",
  "./guide.js",
  "./styles.css",
  "./app.js",
  "./session.js",
  "./manifest.json",
  "./icons/icon-192.png",
  "./icons/icon-512.png",
  "./404.html",
  "./icons/ota-download-green.png"
];

const APP_SHELL_PATHS = new Set(APP_SHELL.map((asset) => new URL(asset, self.location.origin).pathname));

self.addEventListener("message", (event) => {
  const type = event.data && event.data.type;
  if (type === "SKIP_WAITING") {
    self.skipWaiting();
    return;
  }
  if (type === "PURGE_CACHES") {
    event.waitUntil((async () => {
      const keys = await caches.keys();
      await Promise.all(keys.map((k) => caches.delete(k)));
    })());
  }
});

async function precacheShell() {
  const cache = await caches.open(CACHE_NAME);
  await Promise.all(APP_SHELL.map(async (asset) => {
    try {
      const req = new Request(asset, { cache: "reload" });
      const res = await fetch(req);
      if (res && res.ok) await cache.put(asset, res.clone());
    } catch (_) {
      // Ignore optional/missing assets so one bad file does not block activation.
    }
  }));
}

async function pruneCurrentCache() {
  const cache = await caches.open(CACHE_NAME);
  const requests = await cache.keys();
  await Promise.all(requests.map(async (request) => {
    const pathname = new URL(request.url).pathname;
    if (!APP_SHELL_PATHS.has(pathname)) await cache.delete(request);
  }));
}

self.addEventListener("install", (event) => {
  self.skipWaiting();
  event.waitUntil(precacheShell());
});

self.addEventListener("activate", (event) => {
  event.waitUntil((async () => {
    const keys = await caches.keys();
    await Promise.all(keys.map((k) => (k !== CACHE_NAME ? caches.delete(k) : null)));
    await pruneCurrentCache();
    if (self.registration.navigationPreload) {
      try { await self.registration.navigationPreload.enable(); } catch (_) {}
    }
    await self.clients.claim();
  })());
});

async function networkFirst(request, fallbackUrl = null) {
  try {
    const fresh = await fetch(request, { cache: "no-store" });
    if (fresh && fresh.ok) {
      const cache = await caches.open(CACHE_NAME);
      await cache.put(request, fresh.clone());
    }
    return fresh;
  } catch {
    const cached = await caches.match(request, { ignoreSearch: true });
    if (cached) return cached;
    if (fallbackUrl) {
      const fallback = await caches.match(fallbackUrl, { ignoreSearch: true });
      if (fallback) return fallback;
    }
    return Response.error();
  }
}

async function staleWhileRevalidate(request) {
  const cache = await caches.open(CACHE_NAME);
  const cached = await cache.match(request, { ignoreSearch: true });
  const fetchPromise = fetch(request, { cache: "no-store" }).then(async (fresh) => {
    if (fresh && fresh.ok) await cache.put(request, fresh.clone());
    return fresh;
  }).catch(() => cached || Response.error());
  return cached || fetchPromise;
}

async function networkOnlyWithFallback(request, fallbackUrl = null) {
  try {
    return await fetch(request, { cache: "no-store" });
  } catch {
    const cached = await caches.match(request, { ignoreSearch: true });
    if (cached) return cached;
    if (fallbackUrl) {
      const fallback = await caches.match(fallbackUrl, { ignoreSearch: true });
      if (fallback) return fallback;
    }
    return Response.error();
  }
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
    event.respondWith(networkFirst(req));
    return;
  }

  if (sameOrigin && ["image", "font"].includes(req.destination)) {
    event.respondWith(staleWhileRevalidate(req));
    return;
  }

  if (sameOrigin) {
    event.respondWith(networkOnlyWithFallback(req, "./404.html"));
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
