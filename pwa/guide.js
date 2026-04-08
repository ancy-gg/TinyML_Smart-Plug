(function(){
  const tabs = Array.from(document.querySelectorAll('[data-tab]'));
  const panels = Array.from(document.querySelectorAll('[data-panel]'));
  const quickHome = document.getElementById('guideQuickHome');
  const mqMobile = window.matchMedia('(max-width: 920px)');

  function activateTab(name, focusTab){
    const targetTab = tabs.find(tab => tab.dataset.tab === name) || tabs[0];
    const targetName = targetTab?.dataset.tab || tabs[0]?.dataset.tab;
    tabs.forEach((tab, index) => {
      const active = tab.dataset.tab === targetName;
      tab.classList.toggle('is-active', active);
      tab.setAttribute('aria-selected', active ? 'true' : 'false');
      tab.tabIndex = active ? 0 : -1;
      if (focusTab && active) tab.focus({ preventScroll: true });
      tab.setAttribute('id', `tab-${tab.dataset.tab}`);
      tab.setAttribute('aria-posinset', String(index + 1));
      tab.setAttribute('aria-setsize', String(tabs.length));
    });

    panels.forEach((panel) => {
      const active = panel.dataset.panel === targetName;
      panel.classList.toggle('is-active', active);
      panel.hidden = !active && !mqMobile.matches;
      if (mqMobile.matches) panel.hidden = !active;
    });

    try {
      history.replaceState(null, '', `#${targetName}`);
    } catch (_) {}
  }

  tabs.forEach((tab) => {
    tab.addEventListener('click', () => activateTab(tab.dataset.tab, false));
    tab.addEventListener('keydown', (ev) => {
      const idx = tabs.indexOf(tab);
      if (idx < 0) return;
      let next = null;
      if (ev.key === 'ArrowRight') next = tabs[(idx + 1) % tabs.length];
      if (ev.key === 'ArrowLeft') next = tabs[(idx - 1 + tabs.length) % tabs.length];
      if (ev.key === 'Home') next = tabs[0];
      if (ev.key === 'End') next = tabs[tabs.length - 1];
      if (next) {
        ev.preventDefault();
        activateTab(next.dataset.tab, true);
      }
    });
  });

  quickHome?.addEventListener('click', () => activateTab('home', true));

  window.addEventListener('hashchange', () => {
    const name = String(location.hash || '').replace(/^#/, '').trim();
    if (name) activateTab(name, false);
  });

  mqMobile.addEventListener?.('change', () => {
    const active = tabs.find(tab => tab.classList.contains('is-active'))?.dataset.tab || 'home';
    activateTab(active, false);
  });

  const initial = String(location.hash || '').replace(/^#/, '').trim();
  activateTab(initial || 'home', false);
})();
