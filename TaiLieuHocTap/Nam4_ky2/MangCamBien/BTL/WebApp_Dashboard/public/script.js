// --- CẤU HÌNH ---
const firebaseConfig = {
    apiKey: "AIzaSyAH76sndFX2iDnoJq8aiDVBRvyJFerP4Yo",
    databaseURL: "https://espproject-ccd63-default-rtdb.asia-southeast1.firebasedatabase.app/",
};

const TOTAL_NODES = 3;
const NUM_LIGHTS = 4;
const NUM_FANS = 4;
const OFFLINE_THRESHOLD_MINUTES = 10;

// --- KHỞI TẠO FIREBASE ---
firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// --- LẤY CÁC PHẦN TỬ HTML ---
const nodeCardsContainer = document.getElementById('node-cards-container');
const lightsContainer = document.getElementById('lights-control-container');
const fansContainer = document.getElementById('fans-control-container');


// --- LOGIC HIỂN THỊ DỮ LIỆU CẢM BIẾN (Giữ nguyên) ---
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
for (let i = 1; i <= TOTAL_NODES; i++) {
    const nodePath = `/sensorData/node${i}`;
    database.ref(nodePath).on('value', (snapshot) => {
        const data = snapshot.val();
        if (data) {
            document.getElementById(`temp-node${i}`).textContent = data.temperature ? data.temperature.toFixed(1) : 'N/A';
            document.getElementById(`hum-node${i}`).textContent = data.humidity ? data.humidity.toFixed(1) : 'N/A';
            document.getElementById(`light-node${i}`).textContent = data.light_intensity ? data.light_intensity.toFixed(0) : 'N/A';
            document.getElementById(`motion-node${i}`).textContent = data.motion_detected ? "Có người" : "Không có ai";
            updateTimestampAndStatus(i, data.last_update);
        } else {
             updateTimestampAndStatus(i, 0);
        }
    });
}
function updateTimestampAndStatus(nodeId, timestamp) {
    const timeElement = document.getElementById(`time-node${nodeId}`);
    const cardElement = document.getElementById(`node-card-${nodeId}`);
    
    if (timestamp === 0) {
        timeElement.textContent = `Chờ dữ liệu...`;
        cardElement.classList.remove('online', 'offline');
        return;
    }

    const now = Date.now();
    const minutesAgo = (now - timestamp) / 1000 / 60;

    if (minutesAgo > OFFLINE_THRESHOLD_MINUTES) {
        timeElement.textContent = `Mất kết nối`;
        cardElement.classList.add('offline');
        cardElement.classList.remove('online');
    } else {
        cardElement.classList.add('online');
        cardElement.classList.remove('offline');
        if (minutesAgo < 1) {
            timeElement.textContent = "Cập nhật vài giây trước";
        } else {
            timeElement.textContent = `Cập nhật vài phút trước`;// ${Math.round(minutesAgo)} phút trước`;
        }
    }
}

// --- LOGIC ĐIỀU KHIỂN THIẾT BỊ (Cập nhật) ---
function createControlItem(type, id) {
    const item = document.createElement('div');
    item.className = 'control-item'; // Class mới cho mỗi cụm công tắc

    let deviceName, firebasePath;
    if (type === 'lights') {
        deviceName = `Đèn ${id}`;
        firebasePath = `/LED/LED${id}`;
    } else {
        deviceName = `Quạt ${id}`;
        firebasePath = `/LED/Fan${id}`;
    }

    // Cấu trúc HTML mới cho một công tắc
    item.innerHTML = `
        <h4>${deviceName}</h4>
        <label class="switch">
            <input type="checkbox" id="toggle-${type}-${id}">
            <span class="slider round"></span>
        </label>
        <p id="status-${type}-${id}">Trạng thái: TẮT</p>
    `;

    const toggle = item.querySelector(`#toggle-${type}-${id}`);
    const statusSpan = item.querySelector(`#status-${type}-${id}`);

    toggle.addEventListener('change', (e) => {
        database.ref(firebasePath).set(e.target.checked);
    });

    database.ref(firebasePath).on('value', (snapshot) => {
        const state = snapshot.val();
        toggle.checked = state;
        statusSpan.textContent = state ? "Trạng thái: BẬT" : "Trạng thái: TẮT";
    });

    return item;
}

// Tạo các công tắc điều khiển
for (let i = 1; i <= NUM_LIGHTS; i++) { lightsContainer.appendChild(createControlItem('lights', i)); }
for (let i = 1; i <= NUM_FANS; i++) { fansContainer.appendChild(createControlItem('fans', i)); }