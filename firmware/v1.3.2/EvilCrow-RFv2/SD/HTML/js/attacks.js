// EvilCrow RF v2 - Attack Control JavaScript

let attackRunning = false;
let statusUpdateInterval = null;

document.addEventListener('DOMContentLoaded', function() {
    const attackForm = document.getElementById('attack-form');
    const attackTypeSelect = document.getElementById('attack-type');
    const startButton = document.getElementById('start-attack');
    const stopButton = document.getElementById('stop-attack');
    
    // Handle attack type change
    attackTypeSelect.addEventListener('change', updateDynamicParams);
    
    // Handle form submission
    attackForm.addEventListener('submit', function(e) {
        e.preventDefault();
        startAttack();
    });
    
    // Handle stop button
    stopButton.addEventListener('click', stopAttack);
    
    // Initialize dynamic parameters
    updateDynamicParams();
});

function updateDynamicParams() {
    const attackType = document.getElementById('attack-type').value;
    const dynamicParams = document.getElementById('dynamic-params');
    
    let paramsHTML = '';
    
    switch(attackType) {
        case 'MOUSEJACKING':
            paramsHTML = `
                <div class="form-group">
                    <label for="payload-type">Payload Type:</label>
                    <select id="payload-type" name="payloadType">
                        <option value="0">Keyboard</option>
                        <option value="1">Mouse</option>
                        <option value="2">Multimedia</option>
                    </select>
                </div>
            `;
            break;
            
        case 'ROLLJAM':
            paramsHTML = `
                <div class="form-group">
                    <label for="record-time">Record Time (ms):</label>
                    <input type="number" id="record-time" name="recordTime" value="5000">
                </div>
                <div class="form-group">
                    <label for="jam-time">Jam Time (ms):</label>
                    <input type="number" id="jam-time" name="jamTime" value="2000">
                </div>
                <div class="form-group">
                    <label for="replay-count">Replay Count:</label>
                    <input type="number" id="replay-count" name="replayCount" value="3">
                </div>
            `;
            break;
            
        case 'BRUTEFORCE':
            paramsHTML = `
                <div class="form-group">
                    <label for="start-code">Start Code:</label>
                    <input type="number" id="start-code" name="startCode" value="0">
                </div>
                <div class="form-group">
                    <label for="end-code">End Code:</label>
                    <input type="number" id="end-code" name="endCode" value="65535">
                </div>
                <div class="form-group">
                    <label for="code-length">Code Length (bits):</label>
                    <input type="number" id="code-length" name="codeLength" value="16">
                </div>
            `;
            break;
            
        case 'JAMMING':
            paramsHTML = `
                <div class="form-group">
                    <label for="jam-type">Jam Type:</label>
                    <select id="jam-type" name="jamType">
                        <option value="0">Constant</option>
                        <option value="1">Sweep</option>
                        <option value="2">Random</option>
                    </select>
                </div>
                <div class="form-group">
                    <label for="sweep-start">Sweep Start (Hz):</label>
                    <input type="number" id="sweep-start" name="sweepStart">
                </div>
                <div class="form-group">
                    <label for="sweep-end">Sweep End (Hz):</label>
                    <input type="number" id="sweep-end" name="sweepEnd">
                </div>
            `;
            break;
    }
    
    dynamicParams.innerHTML = paramsHTML;
}

function startAttack() {
    const formData = new FormData(document.getElementById('attack-form'));
    const params = new URLSearchParams(formData);
    
    fetch('/api/attack/start', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: params
    })
    .then(response => response.json())
    .then(data => {
        if (data.status === 'Attack started') {
            attackRunning = true;
            document.getElementById('start-attack').disabled = true;
            document.getElementById('stop-attack').disabled = false;
            document.getElementById('attack-status').classList.remove('hidden');
            
            // Start status updates
            statusUpdateInterval = setInterval(updateStatus, 1000);
            
            showMessage('Attack started successfully', 'success');
        } else {
            showMessage('Failed to start attack: ' + (data.error || 'Unknown error'), 'error');
        }
    })
    .catch(error => {
        showMessage('Error starting attack: ' + error.message, 'error');
    });
}

function stopAttack() {
    fetch('/api/attack/stop', {
        method: 'POST'
    })
    .then(response => response.json())
    .then(data => {
        attackRunning = false;
        document.getElementById('start-attack').disabled = false;
        document.getElementById('stop-attack').disabled = true;
        
        // Stop status updates
        if (statusUpdateInterval) {
            clearInterval(statusUpdateInterval);
            statusUpdateInterval = null;
        }
        
        showMessage('Attack stopped', 'info');
    })
    .catch(error => {
        showMessage('Error stopping attack: ' + error.message, 'error');
    });
}

function updateStatus() {
    if (!attackRunning) return;
    
    fetch('/api/attack/status')
    .then(response => response.json())
    .then(data => {
        if (data.running) {
            document.getElementById('current-status').textContent = 'Running';
            document.getElementById('current-action').textContent = data.currentAction || 'Processing...';
            
            // Update progress if available
            if (data.progress) {
                const successRate = data.progress.totalAttempts > 0 ? 
                    (data.progress.successCount / data.progress.totalAttempts * 100).toFixed(1) : 0;
                document.getElementById('success-rate').textContent = successRate + '%';
                
                // Update signal strength
                const signalStrength = data.progress.signalStrength || 0;
                document.getElementById('signal-strength').style.width = signalStrength + '%';
                
                // Update progress bar
                const progress = Math.min(100, (data.progress.totalAttempts / 1000) * 100);
                document.getElementById('attack-progress').style.width = progress + '%';
                document.getElementById('progress-text').textContent = progress.toFixed(1) + '%';
            }
        } else {
            // Attack finished
            attackRunning = false;
            document.getElementById('current-status').textContent = 'Completed';
            document.getElementById('start-attack').disabled = false;
            document.getElementById('stop-attack').disabled = true;
            
            if (statusUpdateInterval) {
                clearInterval(statusUpdateInterval);
                statusUpdateInterval = null;
            }
        }
    })
    .catch(error => {
        console.error('Error updating status:', error);
    });
}

function showMessage(message, type) {
    // Create a simple message display
    const messageDiv = document.createElement('div');
    messageDiv.className = `message message-${type}`;
    messageDiv.textContent = message;
    messageDiv.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        padding: 10px 20px;
        border-radius: 5px;
        color: white;
        z-index: 1000;
        background-color: ${type === 'success' ? '#4CAF50' : type === 'error' ? '#f44336' : '#2196F3'};
    `;
    
    document.body.appendChild(messageDiv);
    
    setTimeout(() => {
        document.body.removeChild(messageDiv);
    }, 3000);
}
