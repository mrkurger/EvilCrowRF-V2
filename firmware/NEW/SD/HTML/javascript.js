// EvilCrow RF v2 - Enhanced JavaScript Functions

let isReceiving = false;
let signalData = [];
let activityLog = [];
let tvBrands = [];

// System Status Functions
function refreshStatus() {
    fetch('/status')
        .then(response => response.text())
        .then(data => {
            document.getElementById('status').textContent = data;
            updateStatusIndicators();
            logActivity('System status refreshed');
        })
        .catch(error => {
            console.error('Error fetching status:', error);
            document.getElementById('status').textContent = 'Error';
            showToast('Failed to refresh status', 'error');
        });
    
    updateSystemMetrics();
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
    signalData = [];
    const display = document.getElementById('signal-display');
    if (display) {
        display.textContent = 'Signal display cleared...';
    }
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
    showToast('🛑 Emergency stop activated!', 'error');
    logActivity('EMERGENCY STOP activated');
    
    // Stop all operations
    isReceiving = false;
    const rxButton = document.getElementById('rx-toggle');
    if (rxButton) {
        rxButton.textContent = 'Start RX';
        rxButton.className = 'btn btn-success';
    }
    
    fetch('/stopjammer', { method: 'POST' });
    fetch('/stoprx', { method: 'POST' });
    
    clearSignalDisplay();
    const results = document.getElementById('analysis-results');
    if (results) {
        results.textContent = 'All operations stopped by emergency stop.';
    }
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

function AutoRefresh(t) {
    setTimeout("window.location.reload(1);", t);
}

function AutoRedirect() {
    window.setTimeout(function () {
        window.location.href = "/";
    }, 5000);
}
