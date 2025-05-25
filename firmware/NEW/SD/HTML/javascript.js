// EvilCrow RF v2 - Professional Enhanced JavaScript
// Version: 2.0.0 - Advanced UI Features

// Global State Management
const AppState = {
    isReceiving: false,
    signalData: [],
    activityLog: [],
    tvBrands: [],
    systemMetrics: {},
    attackProgress: 0,
    currentTheme: 'dark',
    notifications: [],
    isOnline: true,
    lastUpdate: Date.now()
};

// Configuration
const CONFIG = {
    refreshInterval: 30000,
    signalUpdateInterval: 5000,
    maxSignalHistory: 50,
    maxActivityLog: 100,
    toastDuration: 4000,
    animationDuration: 300
};

// Enhanced System Status Functions
function refreshStatus() {
    showLoadingState('status-refresh');

    fetch('/api/status')
        .then(response => {
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            return response.json();
        })
        .then(data => {
            AppState.systemMetrics = data;
            AppState.isOnline = true;
            AppState.lastUpdate = Date.now();

            updateStatusDisplay(data);
            updateStatusIndicators();
            logActivity('System status refreshed', 'info');
            hideLoadingState('status-refresh');
        })
        .catch(error => {
            console.error('Status refresh error:', error);
            AppState.isOnline = false;
            updateOfflineStatus();
            showToast('Failed to refresh status: ' + error.message, 'error');
            hideLoadingState('status-refresh');
        });

    updateSystemMetrics();
}

function updateStatusDisplay(data) {
    const elements = {
        'status': data.status || 'Online',
        'uptime': formatUptime(data.uptime || 0),
        'memory': formatBytes(data.free_heap || 0),
        'clients': data.wifi_clients || 0,
        'frequency': (data.frequency || 433.92) + ' MHz',
        'database-entries': data.database_entries || 0
    };

    Object.entries(elements).forEach(([id, value]) => {
        const element = document.getElementById(id);
        if (element) {
            element.textContent = value;
            element.classList.add('updated');
            setTimeout(() => element.classList.remove('updated'), 500);
        }
    });
}

function updateOfflineStatus() {
    const statusElement = document.getElementById('status');
    if (statusElement) {
        statusElement.textContent = 'Offline';
        statusElement.className = 'status-offline';
    }
}

function updateSystemMetrics() {
    const uptime = Math.floor(Date.now() / 1000 / 60);
    if (document.getElementById('uptime')) {
        document.getElementById('uptime').textContent = uptime + 'm';
    }

    const memory = Math.floor(Math.random() * 50000) + 200000;
    if (document.getElementById('memory')) {
        document.getElementById('memory').textContent = memory;
    }

    const clients = Math.floor(Math.random() * 5) + 1;
    if (document.getElementById('clients')) {
        document.getElementById('clients').textContent = clients;
    }
}

function updateStatusIndicators() {
    if (document.getElementById('status')) {
        document.getElementById('status').textContent = 'Online';
    }
}

// TV Remote Control Functions
function tvControl(action) {
    showToast(`Executing TV ${action}...`, 'info');
    logActivity(`TV ${action} command sent`);

    fetch(`/api/tv/${action}`)
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast(`TV ${action} successful!`, 'success');
                if (document.getElementById('tv-status')) {
                    document.getElementById('tv-status').textContent =
                        `✅ TV ${action} command executed successfully`;
                }
                logActivity(`TV ${action} completed successfully`);
            } else {
                showToast(`TV ${action} failed`, 'error');
                if (document.getElementById('tv-status')) {
                    document.getElementById('tv-status').textContent =
                        `❌ TV ${action} command failed`;
                }
            }
        })
        .catch(error => {
            console.error('TV control error:', error);
            showToast('TV control error', 'error');
            if (document.getElementById('tv-status')) {
                document.getElementById('tv-status').textContent =
                    `❌ Error: ${error.message}`;
            }
        });
}

function tvControlWithBrand(action) {
    const brandElement = document.getElementById('tv-brand');
    const brand = brandElement ? brandElement.value : '';
    const url = brand ? `/api/tv/${action}?brand=${brand}` : `/api/tv/${action}`;

    showToast(`Sending ${action} to ${brand || 'all'} TVs...`, 'info');
    logActivity(`TV ${action} for brand: ${brand || 'universal'}`);

    fetch(url)
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast(`${brand || 'Universal'} TV ${action} successful!`, 'success');
                if (document.getElementById('tv-status')) {
                    document.getElementById('tv-status').textContent =
                        `✅ ${brand || 'Universal'} TV ${action} completed`;
                }
                logActivity(`${brand || 'Universal'} TV ${action} successful`);
            } else {
                showToast(`TV ${action} failed`, 'error');
                if (document.getElementById('tv-status')) {
                    document.getElementById('tv-status').textContent =
                        `❌ ${brand || 'Universal'} TV ${action} failed`;
                }
            }
        })
        .catch(error => {
            console.error('TV control error:', error);
            showToast('TV control error', 'error');
        });
}

function loadTVBrands() {
    fetch('/api/tv/brands')
        .then(response => response.json())
        .then(data => {
            tvBrands = data.brands || [];
            console.log('📺 Loaded TV brands:', tvBrands);
        })
        .catch(error => {
            console.error('Failed to load TV brands:', error);
        });
}

function showTVBrands() {
    if (tvBrands.length > 0) {
        const brandList = tvBrands.join(', ');
        showToast(`Supported brands: ${brandList}`, 'info');
        if (document.getElementById('tv-status')) {
            document.getElementById('tv-status').textContent =
                `📺 Supported brands: ${brandList}`;
        }
    } else {
        showToast('Loading TV brands...', 'info');
        loadTVBrands();
    }
}

function toggleTVControls() {
    const controls = document.getElementById('tv-quick-controls');
    if (controls) {
        controls.classList.toggle('show');
    }
}

// Signal Analysis Functions
function showSignalAnalysis() {
    showToast('Loading signal analysis...', 'info');

    fetch('/api/signal/analyze')
        .then(response => response.json())
        .then(data => {
            const results = document.getElementById('analysis-results');
            if (results) {
                results.innerHTML = `
                    <div style="color: #4CAF50;">📊 Signal Analysis Results:</div>
                    <div>Modulation: ${data.modulation || 'Unknown'}</div>
                    <div>Protocol: ${data.protocol || 'Unknown'}</div>
                    <div>Frequency: ${data.frequency || 'N/A'} MHz</div>
                    <div>Pulse Count: ${data.pulseCount || 0}</div>
                    <div>SNR: ${data.snr || 'N/A'} dB</div>
                    <div>Valid: ${data.isValid ? 'Yes' : 'No'}</div>
                    <div>Description: ${data.description || 'No description'}</div>
                `;
            }
            showToast('Signal analysis complete', 'success');
            logActivity('Signal analysis performed');
        })
        .catch(error => {
            console.error('Analysis error:', error);
            showToast('Analysis failed', 'error');
            const results = document.getElementById('analysis-results');
            if (results) {
                results.textContent = 'Analysis failed: ' + error.message;
            }
        });
}

function showAttackPanel() {
    const attacks = [
        '🚗 Garage Door Brute Force',
        '📺 TV Remote Scan',
        '🔄 Rolling Code Predict',
        '📻 Frequency Sweep'
    ];

    const attackList = attacks.map((attack, index) =>
        `<div style="margin: 5px 0; cursor: pointer; padding: 5px; background: rgba(255,255,255,0.1); border-radius: 4px;"
         onclick="executeAttack(${index})">${attack}</div>`
    ).join('');

    const results = document.getElementById('analysis-results');
    if (results) {
        results.innerHTML = `
            <div style="color: #FF9800;">🔓 Available Attack Tools:</div>
            ${attackList}
            <div style="margin-top: 10px; font-size: 0.8em; color: #b0b0b0;">
                ⚠️ Use responsibly and only on your own devices
            </div>
        `;
    }

    showToast('Attack panel loaded', 'warning');
}

function executeAttack(attackIndex) {
    const attacks = ['garage', 'tv', 'rolling', 'sweep'];
    const attackNames = ['Garage Door', 'TV Remote', 'Rolling Code', 'Frequency Sweep'];

    if (attackIndex < attacks.length) {
        showToast(`Starting ${attackNames[attackIndex]} attack...`, 'warning');
        logActivity(`Attack started: ${attackNames[attackIndex]}`);

        // For TV attack, use the scan function
        if (attackIndex === 1) {
            tvControl('scan');
            return;
        }

        fetch('/api/attack/bruteforce', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ protocol: attackIndex + 1 })
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast(`${attackNames[attackIndex]} attack completed`, 'success');
                logActivity(`Attack completed: ${attackNames[attackIndex]}`);
            } else {
                showToast(`${attackNames[attackIndex]} attack failed`, 'error');
            }
        })
        .catch(error => {
            console.error('Attack error:', error);
            showToast('Attack failed', 'error');
        });
    }
}

// Signal Monitoring Functions
function toggleReceive() {
    const button = document.getElementById('rx-toggle');
    if (!button) return;

    if (isReceiving) {
        isReceiving = false;
        button.textContent = 'Start RX';
        button.className = 'btn btn-success';
        showToast('RX stopped', 'info');
        logActivity('Signal reception stopped');
    } else {
        isReceiving = true;
        button.textContent = 'Stop RX';
        button.className = 'btn btn-danger';
        showToast('RX started', 'success');
        logActivity('Signal reception started');
        updateSignalDisplay();
    }
}

function updateSignalDisplay() {
    if (!isReceiving) return;

    // Simulate signal data
    const timestamp = new Date().toLocaleTimeString();
    const frequency = (433.0 + Math.random() * 2).toFixed(2);
    const rssi = (-60 + Math.random() * 40).toFixed(1);

    const signalLine = `[${timestamp}] ${frequency}MHz RSSI:${rssi}dBm`;
    signalData.push(signalLine);

    if (signalData.length > 20) {
        signalData.shift();
    }

    const display = document.getElementById('signal-display');
    if (display) {
        display.textContent = signalData.join('\n');
    }
}

function clearSignalDisplay() {
    AppState.signalData = [];
    signalData = []; // Legacy compatibility

    const signalDisplay = document.getElementById('signal-display');
    const signalList = document.getElementById('signal-list');
    const signalCount = document.getElementById('signal-count');

    if (signalDisplay) {
        signalDisplay.innerHTML = `
            <div class="signal-header">
                <span class="signal-status">🔍 Ready to scan frequencies...</span>
                <span class="signal-count">Signals: <span id="signal-count">0</span></span>
            </div>
            <div id="signal-list" class="signal-list">
                <!-- Detected signals will appear here -->
            </div>
        `;
    }

    if (signalList) {
        signalList.innerHTML = '';
    }

    if (signalCount) {
        signalCount.textContent = '0';
    }

    logActivity('Signal display cleared', 'info');
    showToast('Signal display cleared', 'info');
}

// Quick Action Functions
function quickReceive() {
    toggleReceive();
}

function quickJammer() {
    showToast('Starting quick jammer...', 'warning');
    logActivity('Quick jammer activated');

    fetch('/startjammer', { method: 'POST' })
        .then(() => {
            showToast('Jammer started', 'success');
            setTimeout(() => {
                fetch('/stopjammer', { method: 'POST' });
                showToast('Jammer stopped', 'info');
                logActivity('Quick jammer stopped');
            }, 5000);
        })
        .catch(error => {
            showToast('Jammer failed', 'error');
        });
}

function emergencyStop() {
    // Confirm emergency stop
    if (!confirm('🚨 EMERGENCY STOP: This will immediately halt all RF operations.\n\nContinue?')) {
        return;
    }

    showToast('🚨 Emergency stop activated', 'warning');
    logActivity('Emergency stop activated', 'warning');

    // Stop all operations
    AppState.isReceiving = false;
    isReceiving = false; // Legacy compatibility
    spectrumAnalyzer.isActive = false;

    // Stop jamming if active
    if (jammingState && jammingState.isActive) {
        stopJamming();
    }

    // Hide spectrum analyzer
    const spectrumContainer = document.getElementById('spectrum-container');
    if (spectrumContainer) {
        spectrumContainer.style.display = 'none';
    }

    // Update UI states
    const rxToggle = document.getElementById('rx-toggle');
    if (rxToggle) {
        rxToggle.textContent = '📥 Start RX';
        rxToggle.className = 'btn btn-success';
    }

    // Clear displays
    clearSignalDisplay();

    const results = document.getElementById('analysis-results');
    if (results) {
        results.innerHTML = `
            <div style="color: #ff5252;">🚨 EMERGENCY STOP ACTIVATED</div>
            <div style="margin-top: 10px;">All RF operations have been stopped.</div>
            <div style="margin-top: 5px; font-size: 0.9em; color: #b0b0b0;">
                System is safe and ready for new operations.
            </div>
        `;
    }

    // Send emergency stop to device
    Promise.all([
        fetch('/api/emergency-stop', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ stop: true })
        }).catch(() => {}),
        fetch('/stopjammer', { method: 'POST' }).catch(() => {}),
        fetch('/stoprx', { method: 'POST' }).catch(() => {}),
        fetch('/api/scanner/stop', { method: 'POST' }).catch(() => {}),
        fetch('/api/spectrum/stop', { method: 'POST' }).catch(() => {})
    ])
    .then(() => {
        showToast('All RF operations stopped', 'success');
        logActivity('Emergency stop completed', 'success');
    })
    .catch(error => {
        console.error('Emergency stop error:', error);
        showToast('Emergency stop commands sent (some may have failed)', 'warning');
    });
}

function loadProtocols() {
    showToast('Loading protocols...', 'info');
    logActivity('Protocol loading requested');

    fetch('/listxmlfiles')
        .then(response => response.text())
        .then(data => {
            const results = document.getElementById('analysis-results');
            if (results) {
                results.innerHTML = data;
            }
            showToast('Protocols loaded', 'success');
        })
        .catch(error => {
            showToast('Failed to load protocols', 'error');
        });
}

// Activity Log Functions
function logActivity(message) {
    const timestamp = new Date().toLocaleTimeString();
    activityLog.unshift({ time: timestamp, message: message });

    if (activityLog.length > 50) {
        activityLog.pop();
    }

    updateActivityDisplay();
}

function updateActivityDisplay() {
    const logElement = document.getElementById('activity-log');
    if (logElement) {
        logElement.innerHTML = activityLog.map(item =>
            `<div class="activity-item">
                <span class="activity-time">${item.time}</span>
                <span class="activity-text">${item.message}</span>
            </div>`
        ).join('');
    }
}

function clearActivity() {
    activityLog = [];
    updateActivityDisplay();
    showToast('Activity log cleared', 'info');
}

// Toast Notification System
function showToast(message, type = 'info') {
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.textContent = message;

    let container = document.getElementById('toast-container');
    if (!container) {
        container = document.createElement('div');
        container.id = 'toast-container';
        container.style.cssText = 'position: fixed; top: 100px; right: 20px; z-index: 1001;';
        document.body.appendChild(container);
    }

    container.appendChild(toast);

    setTimeout(() => toast.classList.add('show'), 100);

    setTimeout(() => {
        toast.classList.remove('show');
        setTimeout(() => {
            if (container.contains(toast)) {
                container.removeChild(toast);
            }
        }, 300);
    }, 3000);
}

// Menu Functions (Legacy)
function updatemenu() {
    const menu = document.getElementById('menu');
    const checkbox = document.getElementById('responsive-menu');

    if (menu && checkbox) {
        if (checkbox.checked) {
            menu.style.borderBottomRightRadius = '0';
            menu.style.borderBottomLeftRadius = '0';
        } else {
            menu.style.borderRadius = '25px';
        }
    }
}

// Enhanced Utility Functions
function formatBytes(bytes) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
}

function formatUptime(milliseconds) {
    const seconds = Math.floor(milliseconds / 1000);
    const minutes = Math.floor(seconds / 60);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);

    if (days > 0) return `${days}d ${hours % 24}h`;
    if (hours > 0) return `${hours}h ${minutes % 60}m`;
    return `${minutes}m`;
}

function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

function throttle(func, limit) {
    let inThrottle;
    return function() {
        const args = arguments;
        const context = this;
        if (!inThrottle) {
            func.apply(context, args);
            inThrottle = true;
            setTimeout(() => inThrottle = false, limit);
        }
    }
}

// Loading State Management
function showLoadingState(elementId) {
    const element = document.getElementById(elementId);
    if (element) {
        element.classList.add('loading-state');
        const spinner = document.createElement('div');
        spinner.className = 'loading loading-sm';
        spinner.id = elementId + '-spinner';
        element.appendChild(spinner);
    }
}

function hideLoadingState(elementId) {
    const element = document.getElementById(elementId);
    if (element) {
        element.classList.remove('loading-state');
        const spinner = document.getElementById(elementId + '-spinner');
        if (spinner) spinner.remove();
    }
}

// Progress Bar Management
function updateProgress(elementId, percentage) {
    const progressBar = document.querySelector(`#${elementId} .progress-bar`);
    if (progressBar) {
        progressBar.style.width = percentage + '%';
        progressBar.setAttribute('aria-valuenow', percentage);
    }
}

// Modal Management
function showModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.classList.add('show');
        document.body.style.overflow = 'hidden';
    }
}

function hideModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.classList.remove('show');
        document.body.style.overflow = '';
    }
}

// Enhanced Error Handling
function handleApiError(error, context = '') {
    console.error(`API Error ${context}:`, error);

    let message = 'An error occurred';
    if (error.message) {
        message = error.message;
    } else if (typeof error === 'string') {
        message = error;
    }

    showToast(`${context ? context + ': ' : ''}${message}`, 'error');
    logActivity(`Error: ${message}`, 'error');
}

// Performance Monitoring
function measurePerformance(name, func) {
    const start = performance.now();
    const result = func();
    const end = performance.now();
    console.log(`${name} took ${(end - start).toFixed(2)} milliseconds`);
    return result;
}

// Local Storage Management
function saveToStorage(key, data) {
    try {
        localStorage.setItem(`evilcrow_${key}`, JSON.stringify(data));
    } catch (error) {
        console.warn('Failed to save to localStorage:', error);
    }
}

function loadFromStorage(key, defaultValue = null) {
    try {
        const data = localStorage.getItem(`evilcrow_${key}`);
        return data ? JSON.parse(data) : defaultValue;
    } catch (error) {
        console.warn('Failed to load from localStorage:', error);
        return defaultValue;
    }
}

// Initialize Enhanced Features
function initializeApp() {
    console.log('🚀 EvilCrow RF v2 Professional UI Initialized');

    // Load saved preferences
    AppState.currentTheme = loadFromStorage('theme', 'dark');

    // Set up auto-refresh with debouncing
    const debouncedRefresh = debounce(refreshStatus, 1000);
    setInterval(debouncedRefresh, CONFIG.refreshInterval);

    // Set up signal monitoring
    setInterval(() => {
        if (AppState.isReceiving) {
            updateSignalDisplay();
        }
    }, CONFIG.signalUpdateInterval);

    // Initialize components
    refreshStatus();
    loadTVBrands();

    // Add keyboard shortcuts
    document.addEventListener('keydown', handleKeyboardShortcuts);

    // Add connection monitoring
    window.addEventListener('online', () => {
        AppState.isOnline = true;
        showToast('Connection restored', 'success');
        refreshStatus();
    });

    window.addEventListener('offline', () => {
        AppState.isOnline = false;
        showToast('Connection lost', 'warning');
    });

    logActivity('System initialized', 'success');
}

function handleKeyboardShortcuts(event) {
    // Ctrl/Cmd + R: Refresh status
    if ((event.ctrlKey || event.metaKey) && event.key === 'r') {
        event.preventDefault();
        refreshStatus();
    }

    // Escape: Close modals
    if (event.key === 'Escape') {
        document.querySelectorAll('.modal.show').forEach(modal => {
            hideModal(modal.id);
        });
    }
}

// Legacy compatibility
function AutoRefresh(t) {
    setTimeout(() => window.location.reload(1), t);
}

function AutoRedirect() {
    setTimeout(() => window.location.href = "/", 5000);
}

// Advanced Signal Scanner Functions
let spectrumAnalyzer = {
    isActive: false,
    canvas: null,
    ctx: null,
    frequencies: [],
    signalStrengths: [],
    detectedSignals: []
};

function startFrequencyScan() {
    const startFreq = parseFloat(document.getElementById('scan-start').value);
    const endFreq = parseFloat(document.getElementById('scan-end').value);

    if (startFreq >= endFreq) {
        showToast('Invalid frequency range', 'error');
        return;
    }

    showToast(`Starting frequency scan: ${startFreq} - ${endFreq} MHz`, 'info');
    logActivity(`Frequency scan started: ${startFreq}-${endFreq} MHz`, 'info');

    // Update UI
    document.querySelector('.signal-status').textContent = `🔍 Scanning ${startFreq} - ${endFreq} MHz...`;

    // Start the scan
    fetch('/api/scanner/start', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            startFreq: startFreq,
            endFreq: endFreq,
            step: 0.1,
            dwellTime: 100
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Frequency scan started', 'success');
            startScanMonitoring();
        } else {
            showToast('Failed to start scan', 'error');
        }
    })
    .catch(error => handleApiError(error, 'Frequency Scan'));
}

function startSpectrumAnalyzer() {
    const container = document.getElementById('spectrum-container');
    const canvas = document.getElementById('spectrum-canvas');

    if (!canvas) {
        showToast('Spectrum canvas not found', 'error');
        return;
    }

    spectrumAnalyzer.canvas = canvas;
    spectrumAnalyzer.ctx = canvas.getContext('2d');
    spectrumAnalyzer.isActive = true;

    container.style.display = 'block';
    showToast('Spectrum analyzer started', 'success');
    logActivity('Spectrum analyzer activated', 'info');

    // Initialize spectrum display
    initializeSpectrumDisplay();

    // Start spectrum updates
    setInterval(updateSpectrumDisplay, 200);
}

function initializeSpectrumDisplay() {
    const ctx = spectrumAnalyzer.ctx;
    const canvas = spectrumAnalyzer.canvas;

    // Set canvas size
    canvas.width = canvas.offsetWidth;
    canvas.height = 200;

    // Clear canvas
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Draw grid
    ctx.strokeStyle = '#333';
    ctx.lineWidth = 1;

    // Vertical grid lines
    for (let i = 0; i <= 10; i++) {
        const x = (canvas.width / 10) * i;
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, canvas.height);
        ctx.stroke();
    }

    // Horizontal grid lines
    for (let i = 0; i <= 5; i++) {
        const y = (canvas.height / 5) * i;
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(canvas.width, y);
        ctx.stroke();
    }
}

function updateSpectrumDisplay() {
    if (!spectrumAnalyzer.isActive || !spectrumAnalyzer.ctx) return;

    fetch('/api/spectrum/data')
        .then(response => response.json())
        .then(data => {
            drawSpectrum(data);
            updateSpectrumInfo(data);
        })
        .catch(error => console.warn('Spectrum update failed:', error));
}

function drawSpectrum(data) {
    const ctx = spectrumAnalyzer.ctx;
    const canvas = spectrumAnalyzer.canvas;

    // Clear previous frame
    ctx.fillStyle = '#000';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Redraw grid
    initializeSpectrumDisplay();

    if (!data.frequencies || data.frequencies.length === 0) return;

    // Draw spectrum
    ctx.strokeStyle = '#00d4ff';
    ctx.lineWidth = 2;
    ctx.beginPath();

    for (let i = 0; i < data.frequencies.length; i++) {
        const x = (canvas.width / data.frequencies.length) * i;
        const rssi = data.rssi[i] || -100;
        const y = canvas.height - ((rssi + 100) / 60) * canvas.height; // Map -100 to -40 dBm

        if (i === 0) {
            ctx.moveTo(x, y);
        } else {
            ctx.lineTo(x, y);
        }
    }
    ctx.stroke();

    // Draw detected signals
    if (data.detectedSignals) {
        data.detectedSignals.forEach(signal => {
            const x = ((signal.frequency - data.startFreq) / (data.endFreq - data.startFreq)) * canvas.width;

            // Draw signal marker
            ctx.fillStyle = '#ff6b35';
            ctx.fillRect(x - 2, 0, 4, canvas.height);

            // Draw signal label
            ctx.fillStyle = '#4caf50';
            ctx.font = '10px monospace';
            ctx.fillText(`${signal.frequency.toFixed(1)}`, x + 5, 15);
        });
    }
}

function updateSpectrumInfo(data) {
    const freqElement = document.getElementById('current-freq');
    const rssiElement = document.getElementById('current-rssi');

    if (freqElement && data.currentFreq) {
        freqElement.textContent = data.currentFreq.toFixed(2);
    }

    if (rssiElement && data.currentRssi) {
        rssiElement.textContent = data.currentRssi.toFixed(1);
    }
}

function startScanMonitoring() {
    const monitorInterval = setInterval(() => {
        fetch('/api/scanner/results')
            .then(response => response.json())
            .then(data => {
                updateSignalList(data.signals || []);

                if (data.completed) {
                    clearInterval(monitorInterval);
                    showToast('Frequency scan completed', 'success');
                    document.querySelector('.signal-status').textContent = '✅ Scan completed';
                }
            })
            .catch(error => {
                console.warn('Scan monitoring error:', error);
                clearInterval(monitorInterval);
            });
    }, 1000);
}

function updateSignalList(signals) {
    const signalList = document.getElementById('signal-list');
    const signalCount = document.getElementById('signal-count');

    if (!signalList) return;

    signalCount.textContent = signals.length;

    signalList.innerHTML = signals.map(signal => `
        <div class="signal-item" onclick="analyzeSignal(${signal.frequency})">
            <div class="signal-frequency">${signal.frequency.toFixed(2)} MHz</div>
            <div class="signal-details">
                RSSI: ${signal.rssi.toFixed(1)} dBm |
                Modulation: ${signal.modulation || 'Unknown'} |
                Protocol: ${signal.protocol || 'Unidentified'}
                ${signal.deviceName ? ` | Device: ${signal.deviceName}` : ''}
            </div>
        </div>
    `).join('');
}

function analyzeSignal(frequency) {
    showToast(`Analyzing signal at ${frequency} MHz...`, 'info');

    fetch(`/api/signal/analyze?freq=${frequency}`)
        .then(response => response.json())
        .then(data => {
            showSignalAnalysisModal(data);
        })
        .catch(error => handleApiError(error, 'Signal Analysis'));
}

// Device Database Functions
let deviceDatabase = {
    devices: [],
    categories: [],
    totalDevices: 0,
    vulnerableDevices: 0,
    unprotectedDevices: 0
};

function searchDeviceDatabase() {
    const searchTerm = document.getElementById('device-search').value;
    const category = document.getElementById('category-filter').value;
    const frequency = document.getElementById('frequency-filter').value;

    showLoadingState('device-search');

    const params = new URLSearchParams();
    if (searchTerm) params.append('search', searchTerm);
    if (category) params.append('category', category);
    if (frequency) params.append('frequency', frequency);

    fetch(`/api/devices/search?${params}`)
        .then(response => response.json())
        .then(data => {
            deviceDatabase.devices = data.devices || [];
            updateDeviceDisplay(data);
            hideLoadingState('device-search');
            showToast(`Found ${data.devices.length} devices`, 'success');
        })
        .catch(error => {
            hideLoadingState('device-search');
            handleApiError(error, 'Device Search');
        });
}

function filterByCategory() {
    searchDeviceDatabase();
}

function filterByFrequency() {
    searchDeviceDatabase();
}

function updateDeviceDisplay(data) {
    const deviceList = document.getElementById('device-list');
    const totalDevices = document.getElementById('total-devices');
    const vulnerableDevices = document.getElementById('vulnerable-devices');
    const unprotectedDevices = document.getElementById('unprotected-devices');

    // Update statistics
    totalDevices.textContent = data.totalCount || data.devices.length;
    vulnerableDevices.textContent = data.vulnerableCount || 0;
    unprotectedDevices.textContent = data.unprotectedCount || 0;

    // Update device list
    if (!data.devices || data.devices.length === 0) {
        deviceList.innerHTML = '<div class="no-results">No devices found. Try adjusting your search criteria.</div>';
        return;
    }

    deviceList.innerHTML = data.devices.map(device => `
        <div class="device-item" onclick="selectDeviceForJamming('${device.name}', ${device.frequency})">
            <div class="device-name">${device.name}</div>
            <div class="device-details">
                <span>📡 ${device.frequency} MHz</span>
                <span>🏭 ${device.manufacturer || 'Unknown'}</span>
                <span>📂 ${device.category}</span>
                <span class="${device.hasRollingCode ? 'device-secure' : 'device-vulnerability'}">
                    ${device.hasRollingCode ? '🔒 Rolling Code' : '🔓 Fixed Code'}
                </span>
            </div>
            ${device.vulnerabilities ? `<div class="device-vulnerability">⚠️ ${device.vulnerabilities}</div>` : ''}
        </div>
    `).join('');
}

function selectDeviceForJamming(deviceName, frequency) {
    const jamTarget = document.getElementById('jam-target');
    const customFreqGroup = document.getElementById('custom-frequency-group');
    const customFreqInput = document.getElementById('custom-jam-freq');

    // Set custom option and frequency
    jamTarget.value = 'custom';
    customFreqInput.value = frequency;
    customFreqGroup.style.display = 'block';

    showToast(`Selected ${deviceName} (${frequency} MHz) for jamming`, 'info');
    logActivity(`Device selected for jamming: ${deviceName} @ ${frequency} MHz`, 'warning');
}

// Jamming Control Functions
let jammingState = {
    isActive: false,
    startTime: 0,
    duration: 0,
    target: null,
    timer: null
};

function startTargetedJamming() {
    const target = document.getElementById('jam-target').value;
    const duration = parseInt(document.getElementById('jam-duration').value);
    const power = document.getElementById('jam-power').value;

    if (!target) {
        showToast('Please select a target device', 'warning');
        return;
    }

    let frequency = null;
    let targetName = '';

    if (target === 'custom') {
        frequency = parseFloat(document.getElementById('custom-jam-freq').value);
        targetName = `Custom (${frequency} MHz)`;

        if (!frequency || frequency < 300 || frequency > 928) {
            showToast('Invalid custom frequency', 'error');
            return;
        }
    } else {
        const targetInfo = getJammingTargetInfo(target);
        frequency = targetInfo.frequencies[0]; // Use first frequency
        targetName = targetInfo.name;
    }

    // Confirm jamming action
    if (!confirm(`⚠️ WARNING: Start jamming ${targetName} for ${duration} seconds?\n\nThis may interfere with legitimate communications and could be illegal in your jurisdiction.`)) {
        return;
    }

    showToast(`Starting jamming: ${targetName}`, 'warning');
    logActivity(`Jamming started: ${targetName} @ ${frequency} MHz for ${duration}s`, 'warning');

    // Start jamming
    fetch('/api/jammer/start', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            frequency: frequency,
            duration: duration,
            power: power,
            target: target
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            startJammingUI(targetName, duration);
        } else {
            showToast('Failed to start jamming', 'error');
        }
    })
    .catch(error => handleApiError(error, 'Jamming Control'));
}

function startJammingUI(targetName, duration) {
    jammingState.isActive = true;
    jammingState.startTime = Date.now();
    jammingState.duration = duration * 1000;
    jammingState.target = targetName;

    // Update UI
    const jammingState_elem = document.querySelector('.jamming-state');
    const jammingProgress = document.getElementById('jamming-progress');
    const jammingLog = document.getElementById('jamming-log');

    jammingState_elem.textContent = `📻 Jamming ${targetName}`;
    jammingState_elem.classList.add('active');
    jammingProgress.style.display = 'block';

    // Add log entry
    addJammingLogEntry(`Jamming started: ${targetName}`, 'warning');

    // Start timer
    jammingState.timer = setInterval(updateJammingProgress, 100);

    // Auto-stop after duration
    setTimeout(() => {
        stopJamming();
    }, jammingState.duration);
}

function updateJammingProgress() {
    if (!jammingState.isActive) return;

    const elapsed = Date.now() - jammingState.startTime;
    const progress = (elapsed / jammingState.duration) * 100;
    const remaining = Math.max(0, Math.ceil((jammingState.duration - elapsed) / 1000));

    // Update progress bar
    updateProgress('jamming-progress', Math.min(100, progress));

    // Update timer
    const timerElement = document.getElementById('jam-timer');
    if (timerElement) {
        timerElement.textContent = `${remaining}s`;
    }

    if (progress >= 100) {
        stopJamming();
    }
}

function stopJamming() {
    if (!jammingState.isActive) return;

    jammingState.isActive = false;

    if (jammingState.timer) {
        clearInterval(jammingState.timer);
        jammingState.timer = null;
    }

    // Update UI
    const jammingState_elem = document.querySelector('.jamming-state');
    const jammingProgress = document.getElementById('jamming-progress');

    jammingState_elem.textContent = '⏹️ Jamming Stopped';
    jammingState_elem.classList.remove('active');
    jammingProgress.style.display = 'none';

    // Add log entry
    addJammingLogEntry('Jamming stopped', 'info');

    // Stop jamming on device
    fetch('/api/jammer/stop', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Jamming stopped', 'info');
                logActivity('Jamming stopped', 'info');
            }
        })
        .catch(error => console.warn('Failed to stop jamming:', error));
}

function previewJammingTarget() {
    const target = document.getElementById('jam-target').value;

    if (!target) {
        showToast('Please select a target first', 'warning');
        return;
    }

    if (target === 'custom') {
        const frequency = document.getElementById('custom-jam-freq').value;
        showToast(`Custom target: ${frequency} MHz`, 'info');
        return;
    }

    const targetInfo = getJammingTargetInfo(target);
    showJammingPreviewModal(targetInfo);
}

function getJammingTargetInfo(target) {
    const targets = {
        'baby-monitors': {
            name: 'Baby Monitors',
            frequencies: [49, 900, 2400],
            description: 'Analog and digital baby monitors',
            warning: 'May affect emergency communications'
        },
        'bluetooth-speakers': {
            name: 'Bluetooth Speakers',
            frequencies: [2400],
            description: 'Bluetooth audio devices',
            warning: 'Will affect all Bluetooth devices in range'
        },
        'wifi-aps': {
            name: 'WiFi Access Points',
            frequencies: [2400, 5000],
            description: 'Wireless network access points',
            warning: 'May disrupt internet connectivity'
        },
        'garage-doors': {
            name: 'Garage Door Openers',
            frequencies: [315, 390, 433.92],
            description: 'Automatic garage door systems',
            warning: 'May affect security systems'
        },
        'car-keys': {
            name: 'Car Key Fobs',
            frequencies: [315, 390, 433.92],
            description: 'Vehicle remote controls',
            warning: 'May affect vehicle security'
        },
        'weather-stations': {
            name: 'Weather Stations',
            frequencies: [433.92, 868, 915],
            description: 'Wireless weather monitoring',
            warning: 'Generally safe to jam'
        },
        'security-systems': {
            name: 'Security Systems',
            frequencies: [315, 433.92, 868],
            description: 'Wireless security devices',
            warning: 'ILLEGAL in most jurisdictions'
        }
    };

    return targets[target] || { name: 'Unknown', frequencies: [], description: '', warning: '' };
}

function addJammingLogEntry(message, type = 'info') {
    const jammingLog = document.getElementById('jamming-log');
    const timestamp = new Date().toLocaleTimeString();

    const entry = document.createElement('div');
    entry.className = `jamming-log-entry ${type}`;
    entry.innerHTML = `<strong>${timestamp}</strong> ${message}`;

    jammingLog.insertBefore(entry, jammingLog.firstChild);

    // Limit log entries
    while (jammingLog.children.length > 20) {
        jammingLog.removeChild(jammingLog.lastChild);
    }
}

// Initialize range slider
document.addEventListener('DOMContentLoaded', function() {
    const durationSlider = document.getElementById('jam-duration');
    const durationValue = document.getElementById('jam-duration-value');
    const jamTarget = document.getElementById('jam-target');
    const customFreqGroup = document.getElementById('custom-frequency-group');

    if (durationSlider && durationValue) {
        durationSlider.addEventListener('input', function() {
            durationValue.textContent = this.value + 's';
        });
    }

    if (jamTarget && customFreqGroup) {
        jamTarget.addEventListener('change', function() {
            customFreqGroup.style.display = this.value === 'custom' ? 'block' : 'none';
        });
    }

    // Load device database on startup
    loadDeviceDatabase();
});

function loadDeviceDatabase() {
    fetch('/api/devices/stats')
        .then(response => response.json())
        .then(data => {
            document.getElementById('total-devices').textContent = data.totalDevices || 'Loading...';
            document.getElementById('vulnerable-devices').textContent = data.vulnerableDevices || '-';
            document.getElementById('unprotected-devices').textContent = data.unprotectedDevices || '-';
        })
        .catch(error => console.warn('Failed to load device stats:', error));
}

// RF Signal Hijacking Functions
let hijackingState = {
    isActive: false,
    mode: '',
    targetType: '',
    capturedSignals: [],
    replayedSignals: 0,
    startTime: 0,
    timeout: 30000,
    sensitivity: 'medium',
    frequencies: [],
    evilTwinActive: false
};

function updateHijackMode() {
    const mode = document.getElementById('hijack-mode').value;
    const modeDescriptions = {
        'passive-learn': 'Safe mode: Only captures and analyzes signals without interference',
        'active-hijack': 'Aggressive mode: Actively jams and replaces signals in real-time',
        'evil-twin': 'Emulates target device to intercept communications',
        'replay-attack': 'Captures signals and replays them with modifications'
    };

    if (mode && modeDescriptions[mode]) {
        addHijackingLogEntry(`Mode selected: ${modeDescriptions[mode]}`, 'info');
    }
}

function updateTargetFrequencies() {
    const targetType = document.getElementById('target-device-type').value;
    const frequencyList = document.getElementById('frequency-list');
    const frequencyContainer = document.getElementById('hijack-frequencies');

    const deviceFrequencies = {
        'car-keys': [315.0, 390.0, 433.92, 868.0],
        'garage-doors': [315.0, 390.0, 433.92],
        'baby-monitors': [49.0, 900.0, 2400.0],
        'security-systems': [315.0, 433.92, 868.0, 915.0],
        'remote-controls': [433.92, 38.0, 56.0],
        'smart-home': [433.92, 868.0, 915.0, 2400.0],
        'weather-stations': [433.92, 868.0, 915.0]
    };

    if (targetType && deviceFrequencies[targetType]) {
        hijackingState.frequencies = deviceFrequencies[targetType];
        frequencyContainer.style.display = 'block';

        frequencyList.innerHTML = hijackingState.frequencies.map(freq =>
            `<span class="frequency-tag" onclick="toggleFrequency(${freq})">${freq} MHz</span>`
        ).join('');

        addHijackingLogEntry(`Target frequencies loaded: ${hijackingState.frequencies.join(', ')} MHz`, 'info');
    } else {
        frequencyContainer.style.display = 'none';
        hijackingState.frequencies = [];
    }
}

function toggleFrequency(frequency) {
    const tags = document.querySelectorAll('.frequency-tag');
    tags.forEach(tag => {
        if (tag.textContent.includes(frequency.toString())) {
            tag.classList.toggle('active');
        }
    });
}

function startSignalHijacking() {
    const mode = document.getElementById('hijack-mode').value;
    const targetType = document.getElementById('target-device-type').value;
    const timeout = parseInt(document.getElementById('capture-timeout').value);
    const sensitivity = document.getElementById('hijack-sensitivity').value;

    if (!mode) {
        showToast('Please select an attack mode', 'warning');
        return;
    }

    if (!targetType) {
        showToast('Please select a target device type', 'warning');
        return;
    }

    // Confirm dangerous operations
    if (mode === 'active-hijack' || mode === 'evil-twin') {
        if (!confirm(`⚠️ WARNING: ${mode.toUpperCase()} mode will actively interfere with RF communications.\n\nThis may be illegal in your jurisdiction and could disrupt legitimate devices.\n\nContinue?`)) {
            return;
        }
    }

    hijackingState.isActive = true;
    hijackingState.mode = mode;
    hijackingState.targetType = targetType;
    hijackingState.timeout = timeout * 1000;
    hijackingState.sensitivity = sensitivity;
    hijackingState.startTime = Date.now();
    hijackingState.capturedSignals = [];
    hijackingState.replayedSignals = 0;

    // Update UI
    const hijackingStateElem = document.querySelector('.hijacking-state');
    const hijackingProgress = document.getElementById('hijacking-progress');
    const capturedSignalsList = document.getElementById('captured-signals-list');

    hijackingStateElem.textContent = `🎭 ${mode.replace('-', ' ').toUpperCase()} Active`;
    hijackingStateElem.className = `hijacking-state ${mode.replace('-', '')}`;
    hijackingProgress.style.display = 'block';
    capturedSignalsList.style.display = 'block';

    addHijackingLogEntry(`Hijacking started: ${mode} on ${targetType}`, 'hijack');
    showToast(`Signal hijacking started: ${mode}`, 'warning');
    logActivity(`RF Hijacking started: ${mode} targeting ${targetType}`, 'warning');

    // Start the hijacking operation
    fetch('/api/hijacking/start', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            mode: mode,
            targetType: targetType,
            frequencies: hijackingState.frequencies,
            timeout: timeout,
            sensitivity: sensitivity
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            startHijackingMonitoring();
        } else {
            stopHijacking();
            showToast('Failed to start hijacking', 'error');
        }
    })
    .catch(error => {
        stopHijacking();
        handleApiError(error, 'Signal Hijacking');
    });
}

function startEvilTwin() {
    const targetType = document.getElementById('target-device-type').value;

    if (!targetType) {
        showToast('Please select a target device type first', 'warning');
        return;
    }

    if (!confirm('⚠️ EVIL TWIN MODE: This will emulate target devices to intercept communications.\n\nThis is an advanced attack that may be illegal. Continue?')) {
        return;
    }

    hijackingState.evilTwinActive = true;

    // Set evil twin mode
    document.getElementById('hijack-mode').value = 'evil-twin';
    updateHijackMode();

    // Start evil twin with specific configuration
    fetch('/api/hijacking/evil-twin', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            targetType: targetType,
            frequencies: hijackingState.frequencies,
            emulateDevices: true,
            interceptMode: true
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            const hijackingStateElem = document.querySelector('.hijacking-state');
            hijackingStateElem.textContent = '👥 EVIL TWIN Active';
            hijackingStateElem.className = 'hijacking-state evil-twin';

            addHijackingLogEntry(`Evil Twin mode activated for ${targetType}`, 'hijack');
            showToast('Evil Twin mode activated', 'warning');
            logActivity(`Evil Twin attack started targeting ${targetType}`, 'warning');

            startEvilTwinMonitoring();
        } else {
            showToast('Failed to start Evil Twin mode', 'error');
        }
    })
    .catch(error => handleApiError(error, 'Evil Twin'));
}

function startHijackingMonitoring() {
    const monitorInterval = setInterval(() => {
        if (!hijackingState.isActive) {
            clearInterval(monitorInterval);
            return;
        }

        // Check for timeout
        const elapsed = Date.now() - hijackingState.startTime;
        if (elapsed >= hijackingState.timeout) {
            clearInterval(monitorInterval);
            stopHijacking();
            return;
        }

        // Update progress
        const progress = (elapsed / hijackingState.timeout) * 100;
        updateProgress('hijacking-progress', progress);

        // Fetch hijacking results
        fetch('/api/hijacking/status')
            .then(response => response.json())
            .then(data => {
                updateHijackingStatus(data);
            })
            .catch(error => console.warn('Hijacking monitoring error:', error));
    }, 1000);
}

function startEvilTwinMonitoring() {
    const monitorInterval = setInterval(() => {
        if (!hijackingState.evilTwinActive) {
            clearInterval(monitorInterval);
            return;
        }

        fetch('/api/hijacking/evil-twin/status')
            .then(response => response.json())
            .then(data => {
                updateEvilTwinStatus(data);
            })
            .catch(error => console.warn('Evil Twin monitoring error:', error));
    }, 2000);
}

function updateHijackingStatus(data) {
    // Update captured signals count
    const capturedCount = document.getElementById('captured-signals');
    const replayedCount = document.getElementById('replayed-signals');

    if (data.capturedSignals) {
        capturedCount.textContent = data.capturedSignals.length;
        hijackingState.capturedSignals = data.capturedSignals;
        updateCapturedSignalsList(data.capturedSignals);
    }

    if (data.replayedCount) {
        replayedCount.textContent = data.replayedCount;
        hijackingState.replayedSignals = data.replayedCount;
    }

    // Add new log entries
    if (data.logEntries) {
        data.logEntries.forEach(entry => {
            addHijackingLogEntry(entry.message, entry.type);
        });
    }
}

function updateEvilTwinStatus(data) {
    if (data.interceptedSignals) {
        data.interceptedSignals.forEach(signal => {
            addHijackingLogEntry(`Intercepted: ${signal.deviceType} signal on ${signal.frequency} MHz`, 'capture');
        });
    }

    if (data.emulatedResponses) {
        data.emulatedResponses.forEach(response => {
            addHijackingLogEntry(`Emulated response to ${response.targetDevice}`, 'hijack');
        });
    }
}

function updateCapturedSignalsList(signals) {
    const container = document.getElementById('signals-container');

    container.innerHTML = signals.map((signal, index) => `
        <div class="captured-signal-item" onclick="selectCapturedSignal(${index})">
            <div class="signal-header">
                <span class="signal-id">Signal #${index + 1}</span>
                <span class="signal-timestamp">${new Date(signal.timestamp).toLocaleTimeString()}</span>
            </div>
            <div class="signal-info">
                <span>📡 ${signal.frequency} MHz</span>
                <span>📊 ${signal.rssi} dBm</span>
                <span>🔧 ${signal.modulation || 'Unknown'}</span>
                <span>📋 ${signal.protocol || 'Unidentified'}</span>
                ${signal.deviceName ? `<span>🏷️ ${signal.deviceName}</span>` : ''}
            </div>
            <div class="signal-actions">
                <button class="btn btn-success" onclick="replaySignal(${index})">🔄 Replay</button>
                <button class="btn btn-info" onclick="analyzeSignal(${signal.frequency})">📊 Analyze</button>
                <button class="btn btn-warning" onclick="modifyAndReplay(${index})">⚡ Modify</button>
            </div>
        </div>
    `).join('');
}

function replayLastCapture() {
    if (hijackingState.capturedSignals.length === 0) {
        showToast('No signals captured yet', 'warning');
        return;
    }

    const lastSignal = hijackingState.capturedSignals[hijackingState.capturedSignals.length - 1];
    replaySignal(hijackingState.capturedSignals.length - 1);
}

function replaySignal(index) {
    if (index >= hijackingState.capturedSignals.length) {
        showToast('Invalid signal index', 'error');
        return;
    }

    const signal = hijackingState.capturedSignals[index];

    addHijackingLogEntry(`Replaying signal #${index + 1} on ${signal.frequency} MHz`, 'replay');
    showToast(`Replaying captured signal #${index + 1}`, 'info');

    fetch('/api/hijacking/replay', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            signalIndex: index,
            signal: signal
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            hijackingState.replayedSignals++;
            document.getElementById('replayed-signals').textContent = hijackingState.replayedSignals;
            addHijackingLogEntry(`Signal #${index + 1} replayed successfully`, 'replay');
            showToast('Signal replayed successfully', 'success');
        } else {
            addHijackingLogEntry(`Failed to replay signal #${index + 1}`, 'error');
            showToast('Signal replay failed', 'error');
        }
    })
    .catch(error => handleApiError(error, 'Signal Replay'));
}

function modifyAndReplay(index) {
    const signal = hijackingState.capturedSignals[index];

    // Show modal for signal modification
    const modifications = prompt(`Modify signal #${index + 1}:\n\nCurrent: ${signal.frequency} MHz\n\nEnter modifications (frequency:power:delay):`);

    if (modifications) {
        const [freq, power, delay] = modifications.split(':');

        fetch('/api/hijacking/modify-replay', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                signalIndex: index,
                modifications: {
                    frequency: parseFloat(freq) || signal.frequency,
                    power: parseInt(power) || 10,
                    delay: parseInt(delay) || 0
                }
            })
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                addHijackingLogEntry(`Modified signal #${index + 1} replayed`, 'replay');
                showToast('Modified signal replayed', 'success');
            } else {
                showToast('Modified replay failed', 'error');
            }
        })
        .catch(error => handleApiError(error, 'Modified Replay'));
    }
}

function stopHijacking() {
    hijackingState.isActive = false;
    hijackingState.evilTwinActive = false;

    // Update UI
    const hijackingStateElem = document.querySelector('.hijacking-state');
    const hijackingProgress = document.getElementById('hijacking-progress');

    hijackingStateElem.textContent = '⏹️ Hijacking Stopped';
    hijackingStateElem.className = 'hijacking-state';
    hijackingProgress.style.display = 'none';

    addHijackingLogEntry('Hijacking operation stopped', 'info');
    showToast('Signal hijacking stopped', 'info');
    logActivity('RF Hijacking stopped', 'info');

    // Stop hijacking on device
    fetch('/api/hijacking/stop', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                addHijackingLogEntry('All hijacking operations terminated', 'info');
            }
        })
        .catch(error => console.warn('Failed to stop hijacking:', error));
}

function addHijackingLogEntry(message, type = 'info') {
    const hijackingLog = document.getElementById('hijacking-log');
    const timestamp = new Date().toLocaleTimeString();

    const entry = document.createElement('div');
    entry.className = `hijacking-log-entry ${type}`;
    entry.innerHTML = `<strong>${timestamp}</strong> ${message}`;

    hijackingLog.insertBefore(entry, hijackingLog.firstChild);

    // Limit log entries
    while (hijackingLog.children.length > 30) {
        hijackingLog.removeChild(hijackingLog.lastChild);
    }
}

// Initialize hijacking controls
document.addEventListener('DOMContentLoaded', function() {
    const captureTimeoutSlider = document.getElementById('capture-timeout');
    const captureTimeoutValue = document.getElementById('capture-timeout-value');

    if (captureTimeoutSlider && captureTimeoutValue) {
        captureTimeoutSlider.addEventListener('input', function() {
            captureTimeoutValue.textContent = this.value + 's';
        });
    }
});

// Attack Dashboard Functions
function startPassiveRecon() {
    showToast('Starting passive reconnaissance...', 'info');
    logActivity('Passive reconnaissance started', 'info');

    // Set passive learning mode
    document.getElementById('hijack-mode').value = 'passive-learn';
    updateHijackMode();

    // Start wide spectrum scan
    document.getElementById('scan-start').value = '300';
    document.getElementById('scan-end').value = '928';
    startFrequencyScan();

    // Add to active operations
    addActiveOperation('Passive Reconnaissance', 'Scanning 300-928 MHz for signals', 'passive-recon');
}

function startCarKeyHijack() {
    showToast('Initiating car key hijacking attack...', 'warning');
    logActivity('Car key hijacking attack started', 'warning');

    // Configure for car key frequencies
    document.getElementById('target-device-type').value = 'car-keys';
    document.getElementById('hijack-mode').value = 'evil-twin';
    updateTargetFrequencies();
    updateHijackMode();

    // Start evil twin mode
    startEvilTwin();

    addActiveOperation('Car Key Hijacking', 'Evil Twin mode on 315/390/433 MHz', 'car-hijack');
}

function startGarageAttack() {
    showToast('Launching garage door attack...', 'warning');
    logActivity('Garage door attack initiated', 'warning');

    // Configure for garage door frequencies
    document.getElementById('target-device-type').value = 'garage-doors';
    document.getElementById('hijack-mode').value = 'replay-attack';
    updateTargetFrequencies();

    // Start signal hijacking
    startSignalHijacking();

    addActiveOperation('Garage Door Attack', 'Capture & replay on 315/390/433 MHz', 'garage-attack');
}

function startBabyMonitorIntercept() {
    showToast('Starting baby monitor interception...', 'warning');
    logActivity('Baby monitor interception started', 'warning');

    // Configure for baby monitor frequencies
    document.getElementById('target-device-type').value = 'baby-monitors';
    document.getElementById('hijack-mode').value = 'active-hijack';
    updateTargetFrequencies();

    // Start active hijacking
    startSignalHijacking();

    addActiveOperation('Baby Monitor Intercept', 'Audio interception on 49/900/2400 MHz', 'baby-monitor');
}

function startWiFiDeauth() {
    showToast('Launching WiFi deauth attack...', 'warning');
    logActivity('WiFi deauth attack started', 'warning');

    // Configure for WiFi jamming
    document.getElementById('jam-target').value = 'wifi-aps';
    document.getElementById('jam-duration').value = '30';
    document.getElementById('jam-power').value = 'high';

    // Start targeted jamming
    startTargetedJamming();

    addActiveOperation('WiFi Deauth Attack', 'DoS on 2.4/5 GHz WiFi bands', 'wifi-deauth');
}

function startSpectrumJam() {
    showToast('Initiating spectrum jamming...', 'danger');
    logActivity('Spectrum jamming attack started', 'warning');

    // Configure for wideband jamming
    document.getElementById('jam-target').value = 'custom';
    document.getElementById('custom-jam-freq').value = '433.92';
    document.getElementById('jam-duration').value = '60';
    document.getElementById('jam-power').value = 'high';

    // Show custom frequency group
    document.getElementById('custom-frequency-group').style.display = 'block';

    // Start jamming
    startTargetedJamming();

    addActiveOperation('Spectrum Jamming', 'Wideband jamming on 433.92 MHz', 'spectrum-jam');
}

function startEvilTwinAttack() {
    showToast('Activating Evil Twin attack...', 'danger');
    logActivity('Evil Twin attack activated', 'warning');

    // Use current target device type or default to car keys
    const targetType = document.getElementById('target-device-type').value || 'car-keys';
    document.getElementById('target-device-type').value = targetType;
    document.getElementById('hijack-mode').value = 'evil-twin';

    updateTargetFrequencies();
    startEvilTwin();

    addActiveOperation('Evil Twin Attack', `Device emulation for ${targetType}`, 'evil-twin');
}

function startMITMAttack() {
    showToast('Launching MITM attack...', 'danger');
    logActivity('MITM attack initiated', 'warning');

    // Configure for MITM
    document.getElementById('hijack-mode').value = 'active-hijack';
    document.getElementById('hijack-sensitivity').value = 'high';

    // Start signal hijacking
    startSignalHijacking();

    addActiveOperation('MITM Attack', 'Signal interception and manipulation', 'mitm-attack');
}

function addActiveOperation(name, description, type) {
    const operationsList = document.getElementById('active-operations-list');
    const noOpsMessage = operationsList.querySelector('.no-operations');

    if (noOpsMessage) {
        noOpsMessage.remove();
    }

    const operationId = `operation-${Date.now()}`;
    const operationElement = document.createElement('div');
    operationElement.className = 'operation-item';
    operationElement.id = operationId;
    operationElement.innerHTML = `
        <div class="operation-info">
            <div class="operation-name">${name}</div>
            <div class="operation-status">${description}</div>
        </div>
        <div class="operation-controls">
            <button class="btn btn-warning" onclick="pauseOperation('${operationId}')">⏸️ Pause</button>
            <button class="btn btn-danger" onclick="stopOperation('${operationId}', '${type}')">🛑 Stop</button>
        </div>
    `;

    operationsList.appendChild(operationElement);

    // Update attack stats
    updateAttackStats();
}

function stopOperation(operationId, type) {
    const operation = document.getElementById(operationId);
    if (operation) {
        operation.remove();
    }

    // Stop the corresponding attack
    switch (type) {
        case 'passive-recon':
        case 'car-hijack':
        case 'garage-attack':
        case 'baby-monitor':
        case 'evil-twin':
        case 'mitm-attack':
            stopHijacking();
            break;
        case 'wifi-deauth':
        case 'spectrum-jam':
            stopJamming();
            break;
    }

    // Check if no operations left
    const operationsList = document.getElementById('active-operations-list');
    if (operationsList.children.length === 0) {
        operationsList.innerHTML = '<div class="no-operations">No active operations - Ready to launch attacks</div>';
    }

    showToast('Operation stopped', 'info');
    updateAttackStats();
}

function pauseOperation(operationId) {
    const operation = document.getElementById(operationId);
    if (operation) {
        const statusElement = operation.querySelector('.operation-status');
        statusElement.textContent = statusElement.textContent.replace('Active', 'Paused');

        // Change pause button to resume
        const pauseBtn = operation.querySelector('.btn-warning');
        pauseBtn.innerHTML = '▶️ Resume';
        pauseBtn.onclick = () => resumeOperation(operationId);
    }

    showToast('Operation paused', 'warning');
}

function resumeOperation(operationId) {
    const operation = document.getElementById(operationId);
    if (operation) {
        const statusElement = operation.querySelector('.operation-status');
        statusElement.textContent = statusElement.textContent.replace('Paused', 'Active');

        // Change resume button back to pause
        const resumeBtn = operation.querySelector('.btn-warning');
        resumeBtn.innerHTML = '⏸️ Pause';
        resumeBtn.onclick = () => pauseOperation(operationId);
    }

    showToast('Operation resumed', 'success');
}

function refreshAttackStats() {
    // Update attack statistics
    fetch('/api/attack/stats')
        .then(response => response.json())
        .then(data => {
            document.getElementById('signals-captured').textContent = data.signalsCaptured || 0;
            document.getElementById('devices-hijacked').textContent = data.devicesHijacked || 0;
            document.getElementById('signals-jammed').textContent = data.signalsJammed || 0;
            document.getElementById('targets-identified').textContent = data.targetsIdentified || 0;
        })
        .catch(error => console.warn('Failed to refresh attack stats:', error));
}

function updateAttackStats() {
    // Increment local stats
    const activeOps = document.querySelectorAll('.operation-item').length;
    document.getElementById('targets-identified').textContent = activeOps;

    // Update other stats based on active operations
    if (hijackingState.capturedSignals.length > 0) {
        document.getElementById('signals-captured').textContent = hijackingState.capturedSignals.length;
    }

    if (hijackingState.replayedSignals > 0) {
        document.getElementById('devices-hijacked').textContent = hijackingState.replayedSignals;
    }
}

function startThreatIntelligence() {
    // Start continuous threat intelligence gathering
    setInterval(() => {
        fetch('/api/threat/intelligence')
            .then(response => response.json())
            .then(data => {
                document.getElementById('detected-devices').textContent = data.detectedDevices || 'Scanning...';
                document.getElementById('vulnerable-targets').textContent = data.vulnerableTargets || '-';
                document.getElementById('active-frequencies').textContent = data.activeFrequencies || '-';
                document.getElementById('signal-strength').textContent = data.signalStrength || '-';
            })
            .catch(error => console.warn('Threat intelligence update failed:', error));
    }, 5000);
}

function startThreatScan() {
    showToast('Starting environmental threat scan...', 'info');
    logActivity('Threat scan initiated', 'info');

    // Start comprehensive scan
    startFrequencyScan();
    startSpectrumAnalyzer();

    // Update threat intelligence immediately
    fetch('/api/threat/scan', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            showToast(`Threat scan completed: ${data.threatsFound || 0} threats found`, 'success');
        })
        .catch(error => handleApiError(error, 'Threat Scan'));
}

function refreshThreatIntel() {
    showToast('Refreshing threat intelligence...', 'info');
    startThreatIntelligence();
}

function exportThreatData() {
    showToast('Exporting threat data...', 'info');

    fetch('/api/threat/export')
        .then(response => response.blob())
        .then(blob => {
            const url = window.URL.createObjectURL(blob);
            const a = document.createElement('a');
            a.href = url;
            a.download = `evilcrow-threat-data-${new Date().toISOString().split('T')[0]}.json`;
            document.body.appendChild(a);
            a.click();
            window.URL.revokeObjectURL(url);
            document.body.removeChild(a);
            showToast('Threat data exported', 'success');
        })
        .catch(error => handleApiError(error, 'Export Threat Data'));
}

function startSignalAnalysis() {
    const mode = document.getElementById('analysis-mode').value;
    const freqStart = document.getElementById('freq-start').value;
    const freqEnd = document.getElementById('freq-end').value;

    showToast(`Starting ${mode} signal analysis...`, 'info');
    logActivity(`Signal analysis started: ${mode} mode`, 'info');

    // Configure scanner for analysis
    document.getElementById('scan-start').value = freqStart;
    document.getElementById('scan-end').value = freqEnd;

    // Start appropriate analysis mode
    switch (mode) {
        case 'passive':
            startPassiveRecon();
            break;
        case 'active':
            startSignalHijacking();
            break;
        case 'deep':
            startSpectrumAnalyzer();
            startFrequencyScan();
            break;
        case 'realtime':
            startSpectrumAnalyzer();
            break;
    }

    addActiveOperation('Signal Analysis', `${mode} analysis on ${freqStart}-${freqEnd} MHz`, 'signal-analysis');
}

function captureSignalSnapshot() {
    showToast('Capturing signal snapshot...', 'info');

    fetch('/api/signal/snapshot', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast(`Snapshot captured: ${data.signalsFound} signals`, 'success');
                logActivity(`Signal snapshot captured: ${data.signalsFound} signals`, 'info');
            } else {
                showToast('Snapshot capture failed', 'error');
            }
        })
        .catch(error => handleApiError(error, 'Signal Snapshot'));
}

function stopAnalysis() {
    emergencyStop();
    showToast('All analysis operations stopped', 'info');
}

// Initialize when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializeApp);
} else {
    initializeApp();
}
