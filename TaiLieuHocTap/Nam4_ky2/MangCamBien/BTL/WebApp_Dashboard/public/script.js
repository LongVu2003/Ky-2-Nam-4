// --- CẤU HÌNH ---
// !!! DÁN FIREBASE CONFIG ĐẦY ĐỦ CỦA BẠN VÀO ĐÂY !!!
const firebaseConfig = {
    apiKey: "AIzaSyAH76sndFX2iDnoJq8aiDVBRvyJFerP4Yo",
    authDomain: "espproject-ccd63.firebaseapp.com",
    databaseURL: "https://espproject-ccd63-default-rtdb.asia-southeast1.firebasedatabase.app/",
    projectId: "espproject-ccd63",
    storageBucket: "espproject-ccd63.appspot.com",
    messagingSenderId: "...", // Điền của bạn
    appId: "..." // Điền của bạn
};

const TOTAL_NODES = 3; 
const NUM_LIGHTS = 4;
const NUM_FANS = 4;

// --- KHỞI TẠO ---
firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// --- LẤY CÁC PHẦN TỬ HTML ---
const nodeCardsContainer = document.getElementById('node-cards-container');
const lightsContainer = document.getElementById('lights-control-container');
const fansContainer = document.getElementById('fans-control-container');
const aiContainer = document.getElementById('ai-control-container');


// --- LOGIC HIỂN THỊ DỮ LIỆU CẢM BIẾN ---
// Tạo card giao diện ban đầu cho các Node
for (let i = 1; i <= TOTAL_NODES; i++) {
    const card = document.createElement('div');
    card.className = 'card';
    card.id = `node-card-${i}`;
    card.innerHTML = `
        <h3>Node ${i}</h3>
        <div class="sensor-data">
            <p><i class="fas fa-thermometer-half"></i>Nhiệt độ: <strong id="temp-node${i}">...</strong> &deg;C</p>
            <p><i class="fas fa-tint"></i>Độ ẩm: <strong id="hum-node${i}">...</strong> %</p>
            <p><i class="fas fa-sun"></i>Ánh sáng: <strong id="light-node${i}">...</strong> lx</p>
            <p><i class="fas fa-running"></i>Chuyển động: <strong id="motion-node${i}">...</strong></p>
        </div>
        <div class="timestamp" id="time-node${i}">Chưa có dữ liệu</div>
    `;
    nodeCardsContainer.appendChild(card);
}

// Lắng nghe dữ liệu từ Firebase cho từng Node
for (let i = 1; i <= TOTAL_NODES; i++) {
    const nodePath = `/sensorData/node${i}`;
    database.ref(nodePath).on('value', (snapshot) => {
        const data = snapshot.val();
        
        // <<< THAY ĐỔI CÁCH GỌI HÀM Ở ĐÂY >>>
        // Thay vì truyền data.last_update, chúng ta truyền cả đối tượng data
        updateNodeCard(i, data);
    });
}

// <<< HÀM ĐÃ ĐƯỢC GỘP VÀ ĐƠN GIẢN HÓA TỐI ĐA >>>
function updateNodeCard(nodeId, data) {
    const timeElement = document.getElementById(`time-node${nodeId}`);
    const cardElement = document.getElementById(`node-card-${nodeId}`);

    // LOGIC ĐƠN GIẢN NHẤT THEO YÊU CẦU CỦA BẠN
    if (data) {
        // Nếu có đối tượng data (không phải null)
        // Cập nhật tất cả thông số
        document.getElementById(`temp-node${nodeId}`).textContent = data.temperature !== undefined ? data.temperature.toFixed(1) : 'N/A';
        document.getElementById(`hum-node${nodeId}`).textContent = data.humidity !== undefined ? data.humidity.toFixed(1) : 'N/A';
        document.getElementById(`light-node${nodeId}`).textContent = data.light_intensity !== undefined ? Math.round(data.light_intensity) : 'N/A';
        document.getElementById(`motion-node${nodeId}`).textContent = data.motion_detected ? "Có người" : "Không có ai";
        
        // Hiển thị trạng thái "Vừa cập nhật"
        timeElement.textContent = `Vừa cập nhật`;
        cardElement.classList.add('online');
        cardElement.classList.remove('offline');

    } else {
        // Nếu không có data (null), hiển thị "Chờ dữ liệu..."
        timeElement.textContent = `Chờ dữ liệu...`;
        cardElement.classList.remove('online', 'offline');
    }
}


// --- LOGIC ĐIỀU KHIỂN THIẾT BỊ ---
function createControlItem(type, id) {
    const item = document.createElement('div');
    item.className = 'control-item';
    let deviceName, firebasePath;
    if (type === 'lights') {
        deviceName = `Đèn ${id}`;
        firebasePath = `/LED/LED${id}`;
    } else {
        deviceName = `Quạt ${id}`;
        firebasePath = `/LED/Fan${id}`;
    }
    item.innerHTML = `<h4>${deviceName}</h4><label class="switch"><input type="checkbox" id="toggle-${type}-${id}"><span class="slider round"></span></label><p id="status-${type}-${id}">Trạng thái: TẮT</p>`;
    const toggle = item.querySelector(`#toggle-${type}-${id}`);
    const statusSpan = item.querySelector(`#status-${type}-${id}`);
    
    toggle.addEventListener('change', (e) => {
        database.ref(firebasePath).set(e.target.checked);
        database.ref('/controls/autoMode').set(false);
    });
    
    database.ref(firebasePath).on('value', (snapshot) => {
        const state = snapshot.val();
        toggle.checked = state;
        statusSpan.textContent = state ? "Trạng thái: BẬT" : "Trạng thái: TẮT";
    });
    return item;
}

// --- LOGIC CHO CHẾ ĐỘ TỰ ĐỘNG ---
function createAutoModeToggle() {
    const item = document.createElement('div');
    item.className = 'control-item';
    item.style.width = '100%';
    item.innerHTML = `
        <div style="display: flex; justify-content: space-around; align-items: center; width: 100%;">
            <div>
                <h4>Kích hoạt AI</h4>
                <p id="status-auto-mode">Trạng thái: TẮT</p>
            </div>
            <label class="switch">
                <input type="checkbox" id="toggle-auto-mode" checked>
                <span class="slider round"></span>
            </label>
        </div>
        <div class="ai-prediction">
            AI Dự Đoán: <strong id="ai-prediction-label">---</strong>
        </div>`;
    const toggle = item.querySelector('#toggle-auto-mode');
    const statusSpan = item.querySelector('#status-auto-mode');
    const predictionLabel = item.querySelector('#ai-prediction-label');
    
    toggle.addEventListener('change', (e) => {
        database.ref('/controls/autoMode').set(e.target.checked);
    });

    database.ref('/controls/autoMode').on('value', (snapshot) => {
        const state = snapshot.val();
        toggle.checked = state;
        statusSpan.textContent = state ? "Trạng thái: ĐANG CHẠY" : "Trạng thái: ĐÃ TẮT";
    });

    database.ref('/ai_status/prediction').on('value', (snapshot) => {
        const prediction = snapshot.val();
        if (prediction) {
            predictionLabel.textContent = prediction.replace(/_/g, ' ');
        } else {
            predictionLabel.textContent = "Chờ dữ liệu...";
        }
    });
    return item;
}

// --- TẠO GIAO DIỆN ---
aiContainer.appendChild(createAutoModeToggle());
for (let i = 1; i <= NUM_LIGHTS; i++) { lightsContainer.appendChild(createControlItem('lights', i)); }
for (let i = 1; i <= NUM_FANS; i++) { fansContainer.appendChild(createControlItem('fans', i)); }