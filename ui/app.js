// Main Vue Application with Hashbang Routing
const { createApp, ref, onMounted, onUnmounted, provide } = Vue;

const app = createApp({
    setup() {
        // ─── Hash Router ──────────────────────────────────────────
        const getHashRoute = () => {
            const hash = window.location.hash.replace('#/', '').replace('#', '');
            return hash || 'dashboard';
        };

        const currentView = ref(getHashRoute());
        const sidebarOpen = ref(false);
        const status = ref(null);

        const navigate = (view) => {
            window.location.hash = `/${view}`;
        };

        const onHashChange = () => {
            currentView.value = getHashRoute();
        };

        // ─── Status Fetching ──────────────────────────────────────
        const fetchStatus = async () => {
            try {
                const res = await fetch('/api/status');
                status.value = await res.json();
            } catch (err) {
                console.error('Failed to fetch status:', err);
            }
        };

        onMounted(() => {
            window.addEventListener('hashchange', onHashChange);
            fetchStatus();
            setInterval(fetchStatus, 10000);
        });

        onUnmounted(() => {
            window.removeEventListener('hashchange', onHashChange);
        });

        // Provide to all components
        provide('status', status);
        provide('currentView', currentView);
        provide('sidebarOpen', sidebarOpen);
        provide('fetchStatus', fetchStatus);
        provide('navigate', navigate);

        return { currentView, sidebarOpen, status, navigate };
    }
});

// Register components
app.component('app-layout', AppLayout);
app.component('sidebar', Sidebar);
app.component('status-card', StatusCard);
app.component('dashboard-view', DashboardView);
app.component('settings-view', SettingsView);

// Mount
app.mount('#app');
