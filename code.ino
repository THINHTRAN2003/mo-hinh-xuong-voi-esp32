// ================= CẤU HÌNH BLYNK =================
#define BLYNK_TEMPLATE_ID "TMPL6Lg9X2Lry" // Khai báo Template ID lấy từ trang web Blynk Cloud
#define BLYNK_TEMPLATE_NAME "mohinhxuong" // Khai báo Tên Template trên dự án Blynk
#define BLYNK_AUTH_TOKEN "2Cj-S8xquP4yhrRWdOX9SpjziA0sU3-Z" // Khai báo Token để xác thực và cấp quyền kết nối với máy chủ Blynk
#define BLYNK_PRINT Serial                // Cấu hình cho phép thư viện Blynk in các thông báo trạng thái mạng ra cổng Serial

// ================= KHAI BÁO THƯ VIỆN =================
#include <WiFi.h>              // Nạp thư viện hỗ trợ giao thức kết nối WiFi cho chip ESP32
#include <WiFiClient.h>        // Nạp thư viện hỗ trợ tạo các kết nối máy khách mạng (TCP/IP)
#include <BlynkSimpleEsp32.h>  // Nạp thư viện điều khiển của Blynk viết riêng cho họ chip ESP32
#include <DHT.h>               // Nạp thư viện giao tiếp với cảm biến nhiệt độ - độ ẩm DHT
#include <Wire.h>              // Nạp thư viện giao tiếp chuẩn I2C (dành cho màn hình LCD)
#include <LiquidCrystal_I2C.h> // Nạp thư viện điều khiển màn hình LCD thông qua module chuyển đổi I2C
#include <SPI.h>               // Nạp thư viện giao tiếp đồng bộ chuẩn SPI (dành cho module RFID)
#include <MFRC522.h>           // Nạp thư viện điều khiển module đầu đọc thẻ từ RFID RC522
#include "soc/soc.h"           // Nạp thư viện quản lý các thanh ghi phần cứng cấp thấp của ESP32
#include "soc/rtc_cntl_reg.h"  // Nạp thư viện cấu hình thanh ghi điều khiển thời gian thực và quản lý nguồn điện

// ================= CẤU HÌNH WIFI =================
char ssid[] = "P1706";         // Khai báo mảng ký tự chứa Tên mạng WiFi (SSID) mà mạch cần bắt
char pass[] = "680677890";     // Khai báo mảng ký tự chứa Mật khẩu của mạng WiFi đó

// ================= CẤU HÌNH CHÂN GIAO TIẾP =================
#define SCK_PIN   18  // Khai báo chân GPIO 18 làm chân xung nhịp (Clock) cho chuẩn SPI
#define MISO_PIN  5   // Khai báo chân GPIO 5 làm chân nhận dữ liệu từ RFID về ESP32 (Master In Slave Out)
#define MOSI_PIN  23  // Khai báo chân GPIO 23 làm chân truyền dữ liệu từ ESP32 sang RFID (Master Out Slave In)
#define SS_PIN    19  // Khai báo chân GPIO 19 làm chân chọn thiết bị (SDA/CS) để kích hoạt RFID
#define RST_PIN   4   // Khai báo chân GPIO 4 làm chân khởi động lại (Reset) cho module RFID
#define I2C_SDA   22  // Khai báo chân GPIO 22 làm chân truyền dữ liệu (SDA) cho chuẩn I2C của LCD
#define I2C_SCL   21  // Khai báo chân GPIO 21 làm chân cấp xung nhịp (SCL) cho chuẩn I2C của LCD
#define DHTPIN    32  // Khai báo chân GPIO 32 nối với dây Data của cảm biến DHT22
#define DHTTYPE   DHT22 // Định nghĩa cho thư viện biết loại cảm biến đang dùng là DHT22 (chính xác hơn DHT11)
#define MQ2_PIN   34  // Khai báo chân GPIO 34 (chỉ đọc Analog) để nhận điện áp từ cảm biến khí Gas MQ-2
#define BTN_WAKE  33  // Khai báo chân GPIO 33 dùng làm chân nối nút nhấn đánh thức ESP32 dậy từ chế độ ngủ

// ================= CẤU HÌNH CHÂN RELAY (CƠ BẮP MẠCH) =================
#define RL_FAN2   26  // Khai báo chân GPIO 26 kích hoạt Relay Quạt 2 (Hút khí rò rỉ)
#define RL_FAN1   27  // Khai báo chân GPIO 27 kích hoạt Relay Quạt 1 (Buồng thổi bụi cửa)
#define RL_DOOR   13  // Khai báo chân GPIO 13 kích hoạt Relay đóng/mở khóa chốt điện từ

// ================= CẤU HÌNH CHÂN CẢNH BÁO (LED, CÒI) =================
#define BUZ_GATE  14  // Khai báo chân GPIO 14 điều khiển còi báo tít tít khi quẹt thẻ
#define BUZ_FIRE  12  // Khai báo chân GPIO 12 điều khiển còi hú liên tục khi báo cháy/rò rỉ Gas
#define LED_XANH  2   // Khai báo chân GPIO 2 điều khiển đèn LED Xanh (Báo hiệu cửa mở/thẻ đúng)
#define LED_DO    15  // Khai báo chân GPIO 15 điều khiển đèn LED Đỏ (Báo hiệu thẻ sai/từ chối)
#define LED_ALM   17  // Khai báo chân GPIO 17 điều khiển đèn LED chớp cảnh báo nguy hiểm rò rỉ khí
#define LED_SYS   16  // Khai báo chân GPIO 16 điều khiển đèn LED sáng lỳ báo trạng thái hệ thống đang có điện

// ================= THÔNG SỐ VẬN HÀNH =================
const String CARD_VALID = "8A 1C D0 35"; // Khai báo hằng số chứa mã UID (Hệ HEX) của chiếc thẻ chủ được phép mở cửa
const int GAS_THRESHOLD = 3000;          // Khai báo ngưỡng điện áp ADC (0-4095) của cảm biến, vượt 3000 là báo động rò rỉ Gas
const unsigned long DOOR_OPEN_TIME = 15000; // Khai báo thời gian duy trì mở cửa là 15 giây (bằng 15000 mili-giây)
const unsigned long MSG_DISPLAY_TIME = 3000; // Khai báo thời gian hiện thông báo chữ lên màn hình LCD là 3 giây (3000ms)

// ================= BIẾN TRẠNG THÁI VÀ ĐỊNH THỜI =================
bool isDoorOpen = false;       // Khởi tạo biến cờ nhớ trạng thái cửa: false là cửa đang đóng, true là cửa đang mở
bool isMsgActive = false;      // Khởi tạo biến cờ báo hiệu màn hình LCD có đang bận hiển thị thông báo hay không
unsigned long timerDoor = 0;   // Khởi tạo biến lưu lại mốc thời gian tại khoảnh khắc cửa bắt đầu mở (để đếm ngược 15s)
unsigned long timerLcdMsg = 0; // Khởi tạo biến lưu lại mốc thời gian lúc LCD bắt đầu hiện chữ thông báo (để đếm ngược 3s)
unsigned long lastSensorTime = 0; // Khởi tạo biến lưu lại mốc thời gian của lần đọc cảm biến gần nhất

// ================= KHỞI TẠO ĐỐI TƯỢNG =================
DHT dht(DHTPIN, DHTTYPE);            // Tạo đối tượng tên 'dht' thuộc lớp DHT để chuẩn bị giao tiếp với cảm biến
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Tạo đối tượng 'lcd', địa chỉ I2C là 0x27, cấu hình màn hình loại 16 cột 2 hàng
MFRC522 mfrc522(SS_PIN, RST_PIN);    // Tạo đối tượng 'mfrc522' gán chân CS và Reset để điều khiển module thẻ từ
BlynkTimer timer;                    // Tạo đối tượng 'timer' của Blynk để thực hiện các tác vụ lặp lại định kỳ mà không treo chip

// ================= HÀM ĐIỀU KHIỂN RELAY CÔNG SUẤT =================
// Hàm bật Relay: Cấu hình chân thành ngõ ra (OUTPUT) và xuất mức logic 0 (LOW) vì board Relay công nghiệp thường kích mức thấp
void batRelay(int pin) { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); }
// Hàm tắt Relay: Cấu hình chân thành ngõ vào (INPUT) đưa vào trạng thái trở kháng cao để ngắt dòng điện kích chân Relay
void tatRelay(int pin) { pinMode(pin, INPUT); }

// ================= HÀM GỬI DỮ LIỆU ĐỊNH KỲ LÊN BLYNK =================
void myTimerEvent() { // Hàm do người dùng tự định nghĩa sẽ được tự động gọi theo chu kỳ
  Blynk.virtualWrite(V0, dht.readTemperature()); // Đọc trị số nhiệt độ hiện tại và đẩy dữ liệu lên chân ảo V0 của App Blynk
  Blynk.virtualWrite(V1, dht.readHumidity());    // Đọc trị số phần trăm độ ẩm hiện tại và đẩy dữ liệu lên chân ảo V1 của App Blynk
  Blynk.virtualWrite(V2, analogRead(MQ2_PIN));   // Đọc số điện áp Analog hiện hành của cảm biến MQ-2 và đẩy lên chân ảo V2
}

// ================= HÀM XỬ LÝ NÚT NHẤN TỪ APP BLYNK =================
BLYNK_WRITE(V4) { // Hàm tự động kích hoạt khi có thao tác gạt/nhấn nút ở chân ảo V4 trên giao diện App điện thoại
  if (param.asInt() == 1 && !isDoorOpen) { // Kiểm tra nếu nút được bật (nhận giá trị 1) VÀ trạng thái cửa hiện tại đang đóng
    lcd.clear(); lcd.setCursor(0, 0); lcd.print(" MO CUA TU XA "); // Xóa LCD, đặt con trỏ viết ở đầu hàng 1 và in chữ thông báo
    
    batRelay(RL_DOOR);              // Gọi hàm cấp điện cho module Relay cửa để đóng mạch cho chốt điện từ tụt vào
    batRelay(RL_FAN1);              // Gọi hàm cấp điện cho module Relay bật Quạt 1 làm buồng thổi bụi cho công nhân
    
    isDoorOpen = true;              // Cập nhật biến trạng thái hệ thống: Cửa đang mở
    timerDoor = millis();           // Lấy mốc thời gian hệ thống ngay lúc này lưu vào biến timerDoor để bắt đầu đếm 15s
    isMsgActive = true;             // Đánh cờ báo hiệu cho vòng lặp chính biết LCD đang bận hiển thị thông báo
    timerLcdMsg = millis();         // Lấy mốc thời gian lúc này để đếm ngược 3 giây trước khi xóa chữ trên màn hình
    Blynk.virtualWrite(V6, "CUA DANG MO"); // Gửi dòng trạng thái dạng Text lên màn hình App Blynk (Chân V6)
  }
}

// ================= HÀM CẤU HÌNH BAN ĐẦU (CHẠY 1 LẦN) =================
void setup() { // Bắt đầu khối hàm setup (chỉ chạy duy nhất 1 lần khi cấp điện hoặc reset)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Vô hiệu hóa thanh ghi tự reset chip khi điện áp sụt (chống ESP32 sập nguồn ảo khi Relay đóng)
  Serial.begin(115200); // Mở cổng giao tiếp dữ liệu nối tiếp (Serial) với máy tính ở tốc độ truyền 115200 baud

  // ĐẢM BẢO TẮT TOÀN BỘ TẢI CÔNG SUẤT KHI MỚI CẮM ĐIỆN
  tatRelay(RL_FAN1); // Tắt Relay Quạt thổi bụi
  tatRelay(RL_FAN2); // Tắt Relay Quạt hút khí Gas
  tatRelay(RL_DOOR); // Tắt Relay Khóa cửa để chốt giữ trạng thái bung ra (khóa chặt)
  
  // CẤU HÌNH VÀ TẮT TOÀN BỘ ĐÈN/CÒI
  pinMode(BUZ_FIRE, OUTPUT); digitalWrite(BUZ_FIRE, LOW); // Đặt chân còi cháy là ngõ ra và tắt mặc định
  pinMode(BUZ_GATE, OUTPUT); digitalWrite(BUZ_GATE, LOW); // Đặt chân còi quẹt thẻ là ngõ ra và tắt mặc định
  pinMode(LED_XANH, OUTPUT); digitalWrite(LED_XANH, LOW); // Đặt chân LED Xanh là ngõ ra và tắt mặc định
  pinMode(LED_DO, OUTPUT);   digitalWrite(LED_DO, LOW);   // Đặt chân LED Đỏ là ngõ ra và tắt mặc định
  pinMode(LED_ALM, OUTPUT);  digitalWrite(LED_ALM, LOW);  // Đặt chân LED chớp cảnh báo là ngõ ra và tắt mặc định
  pinMode(LED_SYS, OUTPUT);  digitalWrite(LED_SYS, HIGH); // Đặt chân LED hệ thống là ngõ ra và bật (HIGH) sáng liên tục báo có nguồn
  pinMode(BTN_WAKE, INPUT_PULLUP); // Cấu hình chân nút bấm tắt máy là ngõ vào có gắn sẵn điện trở kéo lên nguồn (mặc định là HIGH)

  // KHỞI ĐỘNG CÁC CHUẨN GIAO TIẾP VÀ MODULE CẢM BIẾN
  Wire.begin(I2C_SDA, I2C_SCL);   // Khởi động giao tiếp phần cứng I2C với 2 chân SDA và SCL quy định
  lcd.init();                     // Khởi tạo các thanh ghi bên trong màn hình LCD
  lcd.backlight();                // Ra lệnh bật sáng dải đèn nền phía sau màn hình LCD
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN); // Khởi động bus giao tiếp SPI để chuẩn bị nói chuyện với đầu đọc RFID
  mfrc522.PCD_Init();             // Gửi cấu hình khởi tạo cài đặt tần số và độ nhạy cho module đọc thẻ RFID RC522
  dht.begin();                    // Khởi động cảm biến nhiệt độ - độ ẩm để nó bắt đầu thu thập số liệu

  // KẾT NỐI MẠNG VÀ CÀI ĐẶT THỜI GIAN
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Bắt đầu nỗ lực kết nối vào mạng WiFi và đăng nhập vào máy chủ Blynk bằng Token
  timer.setInterval(2000L, myTimerEvent);    // Thiết lập cho bộ định thời (Timer) tự động gọi hàm myTimerEvent đều đặn mỗi 2000ms

  // KIỂM TRA LÝ DO CHIP KHỞI ĐỘNG (BẬT NÚT HAY CẮM ĐIỆN)
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) { // Kiểm tra hàm gọi nguyên nhân thức dậy, nếu là thức từ giấc ngủ sâu
    lcd.print("  HE THONG: BAT "); // In ra màn hình dòng chữ báo hiệu mạch vừa được bật lên từ nút nhấn
    while(digitalRead(BTN_WAKE) == LOW) { delay(10); } // Tạo vòng lặp kẹt để đợi cho tới khi ngón tay người dùng buông nút nhấn ra hoàn toàn
  } else { // Nếu thức dậy bằng cách vừa cắm phích cắm điện vào
    lcd.print("  KHOI DONG...  "); // In ra màn hình dòng chữ báo hiệu đang nạp hệ điều hành
  }
  delay(1500); // Tạm dừng mọi thứ trong 1.5 giây để người dùng kịp quan sát các thông báo trên
  lcd.clear(); // Ra lệnh cho LCD xóa sạch toàn bộ ký tự trên màn hình để chuẩn bị vào vòng lặp chính
}

// ================= HÀM VÒNG LẶP CHÍNH (CHẠY LIÊN TỤC VÔ HẠN) =================
void loop() { // Bắt đầu khối hàm loop
  Blynk.run(); // Hàm nòng cốt của Blynk, phải gọi liên tục để duy trì sóng WiFi và nhận dữ liệu điều khiển từ điện thoại
  timer.run(); // Kiểm tra thời gian hệ thống liên tục xem đã đến lúc (2 giây) phải kích hoạt hàm gửi dữ liệu lên App chưa

  // ================= 1. BỘ PHẬN XỬ LÝ ĐẦU ĐỌC THẺ TỪ RFID =================
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) { // Kiểm tra đồng thời: Đang có thẻ ở gần anten? VÀ đọc thẻ đó thành công?
    String uid = ""; // Khởi tạo một biến dạng chuỗi (String) trống để nối mã thẻ vào
    for (byte i = 0; i < mfrc522.uid.size; i++) { // Chạy vòng lặp quét qua số lượng byte dữ liệu trong mã UID của thẻ đó (thường là 4 byte)
      uid += (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "); // Nếu mã thập phân của byte đó nhỏ hơn 16, tự động chèn thêm số "0" vào trước để đủ định dạng 2 ký tự
      uid += String(mfrc522.uid.uidByte[i], HEX); // Đổi byte số liệu sang định dạng chữ Hệ Thập Lục Phân (HEX) và nối đuôi vào chuỗi uid
    }
    uid.toUpperCase(); // Dùng hàm đổi toàn bộ các chữ cái cái thường trong mã thẻ (nếu có) thành in hoa (VD: a -> A)
    uid.trim();        // Dùng hàm cắt gọt để loại bỏ khoảng trắng thừa ở 2 đầu chuỗi, tránh lỗi so sánh

    lcd.clear(); // Quét dọn sạch màn hình LCD
    lcd.setCursor(0, 0); // Di chuyển con trỏ viết lên cột 0, hàng 0 (Góc trên cùng bên trái)
    
    // TIẾN HÀNH SO SÁNH MÃ THẺ:
    if (uid == CARD_VALID) { // So sánh trực tiếp chuỗi mã vừa đọc được với chuỗi mã thẻ đúng đã lưu trên đầu code
      // === NẾU KẾT QUẢ: THẺ HỢP LỆ ===
      digitalWrite(BUZ_GATE, HIGH); delay(150); digitalWrite(BUZ_GATE, LOW); // Xuất điện áp cao cho còi 150ms rồi tắt (Tạo 1 tiếng BÍP dứt khoát)
      digitalWrite(LED_XANH, HIGH); // Bật đèn LED màu xanh lá cây báo hiệu hệ thống chấp nhận thẻ
      lcd.print("   THE HOP LE   "); // In dòng chữ thông báo hợp lệ lên màn hình LCD
      
      batRelay(RL_DOOR);            // Ra lệnh mở điện Relay để chốt cửa thu vào
      batRelay(RL_FAN1);            // Ra lệnh mở điện Relay Quạt 1 để thổi bụi khởi động
      
      isDoorOpen = true;            // Cập nhật lại bộ nhớ trạng thái cửa thành "đang mở"
      timerDoor = millis();         // Lấy mốc đếm giờ hiện tại lưu lại để canh thời gian 15 giây đóng cửa
      Blynk.virtualWrite(V6, "THE HOP LE"); // Đẩy trực tiếp dòng thông báo Text lên chân ảo V6 của Blynk App
    } else {
      // === NẾU KẾT QUẢ: SAI THẺ (NGƯỜI LẠ) ===
      lcd.print("    SAI THE!    "); // In dòng chữ báo sai thẻ lên LCD
      
      digitalWrite(BUZ_GATE, HIGH); digitalWrite(LED_DO, HIGH); delay(150); // Nhịp 1: Bật còi báo và Đèn Đỏ sáng cùng lúc trong 150ms
      digitalWrite(BUZ_GATE, LOW);  digitalWrite(LED_DO, LOW);  delay(100); // Nhịp 2: Tắt còi báo và Đèn Đỏ để nghỉ nhịp trong 100ms
      digitalWrite(BUZ_GATE, HIGH); digitalWrite(LED_DO, HIGH); delay(150); // Nhịp 3: Lặp lại bật còi báo và Đèn Đỏ thêm 150ms
      digitalWrite(BUZ_GATE, LOW);  digitalWrite(LED_DO, LOW);              // Nhịp 4: Tắt hẳn hiệu ứng báo động sai thẻ
      
      Blynk.virtualWrite(V6, "SAI THE!"); // Đẩy cảnh báo báo động xâm nhập lên chân ảo V6 của Blynk App
    }
    isMsgActive = true;      // Đánh cờ báo hiệu cho toàn hệ thống biết màn hình LCD đang bận hiện thông báo thẻ
    timerLcdMsg = millis();  // Lấy mốc thời gian hiện tại lưu lại để canh 3 giây sau tiến hành xóa màn hình
    mfrc522.PICC_HaltA();    // Ra lệnh cho chip RFID gửi xung ru ngủ chiếc thẻ vừa quẹt, ép nó ngừng phát tín hiệu để tránh đọc lặp lại nhiều lần
  }

  // ================= 2. BỘ PHẬN XÓA THÔNG BÁO TẠM TRÊN LCD =================
  if (isMsgActive && (millis() - timerLcdMsg >= MSG_DISPLAY_TIME)) { // Nếu LCD đang bận VÀ thời gian trôi qua tính từ lúc hiện thông báo đã lớn hơn/bằng 3 giây
    isMsgActive = false; // Thu hồi cờ báo bận, trả lại màn hình LCD cho trạng thái rảnh rỗi
    lcd.clear();         // Xóa trống màn hình LCD để chuẩn bị cho việc in nhiệt độ/độ ẩm định kỳ
  }

  // ================= 3. BỘ PHẬN TỰ ĐỘNG ĐÓNG CỬA SAU 15S =================
  if (isDoorOpen && (millis() - timerDoor >= DOOR_OPEN_TIME)) { // Nếu cửa hiện tại đang báo mở VÀ thời gian mở đã trôi qua lớn hơn/bằng 15 giây
    isDoorOpen = false;       // Cập nhật lại bộ nhớ trạng thái cửa thành "đã đóng"
    
    tatRelay(RL_DOOR);        // Ngắt dòng điện khỏi Relay, lò xo trong khóa từ sẽ tự động đẩy chốt ra khóa cửa lại
    tatRelay(RL_FAN1);        // Ngắt điện Relay Quạt 1 để tắt tính năng buồng thổi bụi bảo vệ động cơ
    
    digitalWrite(LED_XANH, LOW); // Tắt đèn LED Xanh báo hiệu cửa mở
    Blynk.virtualWrite(V4, 0);   // Gửi lệnh ảo có giá trị 0 lên nút bấm V4 để nút bấm trên điện thoại tự động gạt về trạng thái tắt
  }

  // ================= 4. BỘ PHẬN ĐỌC CẢM BIẾN MÔI TRƯỜNG & PHÁT HIỆN RÒ RỈ GAS =================
  if (millis() - lastSensorTime > 1500) { // Kiểm tra nếu khoảng thời gian trôi qua từ lần đọc trước đã lớn hơn 1500ms (1.5 giây) - Phương pháp thay thế hàm delay()
    lastSensorTime = millis();       // Ghi nhận mốc thời gian lúc này vào biến để bắt đầu canh giờ cho chu kỳ đọc 1.5 giây tiếp theo
    float t = dht.readTemperature(); // Ra lệnh cho cảm biến DHT22 lấy mẫu nhiệt độ và trả về lưu vào biến số thực (float) tên là 't'
    int gas = analogRead(MQ2_PIN);   // Ra lệnh đọc chuyển đổi tín hiệu Tương tự sang Số (ADC) của cảm biến Gas lưu vào biến số nguyên tên 'gas'

    if (!isMsgActive) { // CHỈ THỰC HIỆN đoạn in hiển thị số liệu nếu màn hình LCD đang rảnh (không bận thông báo quẹt thẻ)
      lcd.setCursor(0, 0); // Di chuyển con trỏ in ra vị trí cột 0, hàng 0
      lcd.print("T:"); // In tiền tố báo hiệu cho Nhiệt độ
      lcd.print(isnan(t) ? 0 : t, 1); // Dùng toán tử ba ngôi: Nếu giá trị đọc bị lỗi (isnan) thì in số 0. Ngược lại in giá trị nhiệt độ, làm tròn lấy 1 số sau dấu phẩy
      lcd.print(" G:"); // In tiền tố báo hiệu số liệu khí Gas
      lcd.print(gas); // In nguyên xi giá trị điện áp số từ 0 - 4095 của khí gas lên màn hình
      lcd.print("    "); // In 4 khoảng trắng ra cuối câu để xóa đè lên các ký tự rác của thông báo cũ nếu có
      
      lcd.setCursor(0, 1); // Di chuyển con trỏ in ra vị trí cột 0, hàng 1 (xuống dòng)
      lcd.print("HDo: "); // In tiền tố viết tắt cho Độ ẩm (Humidity)
      lcd.print(dht.readHumidity(), 0); // Đọc thông số độ ẩm từ DHT22 và in ra làm tròn thành số nguyên (0 chữ số sau dấu phẩy)
      lcd.print("%       "); // In thêm biểu tượng phần trăm % và một loạt khoảng trắng nối đuôi để xóa rác hiển thị
    }

    // --- KIỂM TRA ĐIỀU KIỆN CHÁY NỔ / CẢNH BÁO GAS ---
    if (gas > GAS_THRESHOLD) { // So sánh nếu số điện áp đo được lớn hơn mức 3000 (Nguy hiểm cháy)
      digitalWrite(BUZ_FIRE, HIGH); // Cấp điện áp cao (HIGH) kích còi báo cháy rú lên liên tục
      digitalWrite(LED_ALM, HIGH);  // Cấp điện áp cao (HIGH) kích đèn LED chớp báo cháy/nguy hiểm sáng rực lên
      batRelay(RL_FAN2);            // Bật ngay lập tức Relay của Quạt hút 2 để đẩy khí độc ra khỏi phòng
      Blynk.virtualWrite(V5, 1);    // Kích hoạt sáng đèn cảnh báo khẩn cấp trên giao diện điện thoại (chân ảo V5)
      Blynk.virtualWrite(V6, "CANH BAO GAS!"); // Đổi chuỗi text trạng thái V6 thành thông điệp cảnh báo đỏ
    } else { // Nếu số điện áp đo được nhỏ hơn hoặc bằng 3000 (An toàn)
      digitalWrite(BUZ_FIRE, LOW);  // Ngắt điện còi báo cháy
      digitalWrite(LED_ALM, LOW);   // Ngắt điện đèn cảnh báo rò rỉ khí
      tatRelay(RL_FAN2);            // Ngắt điện Relay Quạt hút 2 vì không có khí độc
      Blynk.virtualWrite(V5, 0);    // Tắt đèn cảnh báo khẩn cấp trên giao diện điện thoại
    }
  }

  // ================= 5. BỘ PHẬN XỬ LÝ NÚT NHẤN CHẾ ĐỘ NGỦ SÂU TẾT KIỆM PIN =================
  if (digitalRead(BTN_WAKE) == LOW) { // Đọc trạng thái chân GPIO 33, nếu thấy đang ở mức 0 (Nút bị người dùng nhấn xuống đất)
    delay(50); // Cố tình dừng hệ thống 50ms nhằm mục đích chống nhiễu phần cứng do lá đồng trong nút nảy lên nảy xuống
    if (digitalRead(BTN_WAKE) == LOW) { // Đọc lại lần 2, nếu vẫn chắc chắn ngón tay người dùng vẫn đang đè nút
      tatRelay(RL_FAN1);  // Lệnh cắt ngắt an toàn dòng điện đi vào Quạt thổi bụi
      tatRelay(RL_FAN2);  // Lệnh cắt ngắt an toàn dòng điện đi vào Quạt hút khí gas
      tatRelay(RL_DOOR);  // Lệnh cắt ngắt an toàn dòng điện khóa chốt
      Blynk.virtualWrite(V6, "TAT MAY"); // Báo thông điệp về điện thoại cho biết sa bàn đã bị người dùng nhấn tắt
      
      // Lệnh gọi hàm cấu hình đánh thức: Chọn chân GPIO 33 là nơi tiếp nhận tín hiệu, đánh thức ESP khi mức logic chạm 0 (LOW)
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 0); 
      esp_deep_sleep_start(); // Ra lệnh cho bộ vi xử lý ngừng cấp xung nhịp, đưa bộ nhớ, chip mạng và vi xử lý vào trạng thái ngủ sâu Deep Sleep
    }
  }
} // Dấu ngoặc nhọn kết thúc toàn bộ cấu trúc hàm vòng lặp loop()