// --- CẤU HÌNH ---
// Tôi đã điền sẵn API Key và URL bạn cung cấp, bạn chỉ cần kiểm tra lại.
const firebaseConfig = {
    apiKey: "AIzaSyAH76sndFX2iDnoJq8aiDVBRvyJFerP4Yo",
    databaseURL: "https://espproject-ccd63-default-rtdb.asia-southeast1.firebasedatabase.app/",
};

const TOTAL_NODES = 3; // Tổng số node cảm biến bạn có
const NUM_LIGHTS = 4;
const NUM_FANS = 4;
const OFFLINE_THRESHOLD_MINUTES = 10; // Sau 10 phút không có dữ liệu mới, node sẽ bị coi là offline

// --- KHỞI TẠO FIREBASE ---
firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// --- LẤY CÁC PHẦN TỬ HTML ---
const nodeCardsContainer = document.getElementById('node-cards-container');
const lightsGrid = document.getElementById('lights-control-grid');
const fansGrid = document.getElementById('fans-control-grid');


// ===================================================================
// PHẦN MỚI: LOGIC HIỂN THỊ DỮ LIỆU CẢM BIẾN
// ===================================================================

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
        
        if (data) {
            // Cập nhật các giá trị lên giao diện
            document.getElementById(`temp-node${i}`).textContent = data.temperature ? data.temperature.toFixed(1) : 'N/A';
            document.getElementById(`hum-node${i}`).textContent = data.humidity ? data.humidity.toFixed(1) : 'N/A';
            document.getElementById(`light-node${i}`).textContent = data.light_intensity ? data.light_intensity.toFixed(0) : 'N/A';
            document.getElementById(`motion-node${i}`).textContent = data.motion_detected ? "Có người" : "Không có ai";

            // Cập nhật thời gian và kiểm tra trạng thái Online/Offline
            updateTimestampAndStatus(i, data.last_update);
        } else {
            // Xử lý trường hợp node chưa có dữ liệu trên Firebase
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
            timeElement.textContent = `Cập nhật ${Math.round(minutesAgo)} phút trước`;
        }
    }
}


// ===================================================================
// PHẦN CŨ: LOGIC ĐIỀU KHIỂN THIẾT BỊ (Giữ nguyên như code của bạn)
// ===================================================================

function createControlCard(type, id) {
    const card = document.createElement('div');
    card.className = 'control-card';
    let deviceName, firebasePath;
    if (type === 'lights') {
        deviceName = `Đèn ${id}`;
        firebasePath = `/LED/LED${id}`;
    } else {
        deviceName = `Quạt ${id}`;
        firebasePath = `/LED/Fan${id}`;
    }

    card.innerHTML = `
        <h3>${deviceName}</h3>
        <label class="switch">
            <input type="checkbox" id="toggle-${type}-${id}">
            <span class="slider round"></span>
        </label>
        <p id="status-${type}-${id}">Đang tải...</p>
    `;

    const toggle = card.querySelector(`#toggle-${type}-${id}`);
    const statusSpan = card.querySelector(`#status-${type}-${id}`);

    toggle.addEventListener('change', (e) => {
        database.ref(firebasePath).set(e.target.checked);
        console.log(`Da gui lenh: ${firebasePath} -> ${e.target.checked}`);
    });

    database.ref(firebasePath).on('value', (snapshot) => {
        const state = snapshot.val();
        toggle.checked = state;
        statusSpan.textContent = state ? "Trạng thái: BẬT" : "Trạng thái: TẮT";
    });

    return card;
}

// Tạo các công tắc điều khiển
for (let i = 1; i <= NUM_LIGHTS; i++) { lightsGrid.appendChild(createControlCard('lights', i)); }
for (let i = 1; i <= NUM_FANS; i++) { fansGrid.appendChild(createControlCard('fans', i)); }