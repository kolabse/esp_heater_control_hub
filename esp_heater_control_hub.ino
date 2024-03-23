/*

*/
// Иконки элементов управления

#define ICO_APP "f185"          // иконка устройства
#define ICO_AUTO "f6e8"         // автоматический режим
#define ICO_CALEND "f073"       // календарь
#define ICO_CLK "f017"          // часы
#define ICO_COOLANT "f773"      // теплоноситель
#define ICO_DASH "f3fd"         // приборная панель
#define ICO_ECO "f4d8"          // энергосбережение
#define ICO_FIRE "f7e4"         // нагрев
#define ICO_INDOOR "f015"       // в помещении
#define ICO_LGHT "f0eb"         // освещение
#define ICO_MAGIC "f0d0"        // умный режим
#define ICO_MANUAL "f256"       // ручной режим
#define ICO_MINUS "f068"        // уменьшить
#define ICO_PRCNT "f541"        // процент
#define ICO_PLUS "f067"         // увеличить
#define ICO_PWR "f011"          // включить/выключить
#define ICO_SAVE "f0c7"         // сохранить
#define ICO_STNGS "f085"        // настройки
#define ICO_SLDR "f1de"         // слайдеры
#define ICO_SNOW "f2dc"         // холодно (снежинка)
#define ICO_STOP "f04d"         // остановить
#define ICO_TEMP_HI "f769"      // градусник высокая температура
#define ICO_TEMP_LO "f76b"      // градусник низкая температура
#define ICO_TERM_000 "f2cb"     // градусник 0%
#define ICO_TEMP_025 "f2ca"     // градусник 25%
#define ICO_TEMP_050 "f2c9"     // градусник 50%
#define ICO_TEMP_075 "f2c8"     // градусник 75%
#define ICO_TEMP_100 "f2c7"     // градусник 100%
#define ICO_WIFI "f1eb"         // уровень сигнала

// периоды мигания светодиодом в зависимости от режима работы

#define WIFI_STA_BLINK 2000
#define WIFI_AP_BLINK 800
#define WIFI_BLINK_ON 100

#include <Arduino.h>
#include <GyverHub.h>
GyverHub hub;

#include <PairsFile.h>
PairsFile wifiConfig(&GH_FS, "/config/wifi.dat", 3000);

#include <Blinker.h>
Blinker led(LED_BUILTIN);

static String wifi_SSID;
static String wifi_PASS;
static uint8_t wifi_connect_attempts = 10;
static bool cnfWiFiUpd;

// билдер
void build(gh::Builder& b) {
    static byte tab;
    b.Title_("mainHeader", "Нагреватель").size(3);
    if (b.Tabs(&tab).text("Состояние;Параметры;Настройки").noLabel().noTab().click()) b.refresh();

    b.show(tab == 0);

    // Вкладка "Состояние"

    // Состоянте (греем/ждем)
    // Режим (ручной/авто/умный)
    // Требуемая температура помещения
    // Фактическая температура помещения
    // Фактическая температура прибора отопления
    // График температур за последние сутки/недлелю/месяц*
    // Быстрое изменение температуры
    // Включение/выключение
    b.Spinner();

    b.show(tab == 1);

    // Вкладка "Параметры"

    // Настройки режима "АВТО"
    // Настройки режима "Умный"
    // Настройки предельных значений

    b.Slider();

    b.show(tab == 2);

    // Вкладка "Настройки"

    // Форма ввода настроек wifi
    b.beginRow();
    
    b.beginCol(2);
    b.Label("SSID").noLabel().noTab().fontSize(17).align(gh::Align::Left);
    b.Label("Password").noLabel().noTab().fontSize(17).align(gh::Align::Left);
    b.endCol();
    
    b.beginCol(2);
    b.Input_(F("ssid"), &wifi_SSID).noLabel().noTab();
    b.Pass_(F("pass"), &wifi_PASS).noLabel().noTab();
    b.endCol();

    b.beginCol(1);

    // Модальное окно подтверждения сохранения новых настроек wifi
    if (b.Confirm_("wifiUpd", &cnfWiFiUpd).text("Заменить настройки точки доступа?").click()) {
        if (cnfWiFiUpd) {
            wifiConfig.set("ssid", wifi_SSID);
            wifiConfig.set("pass", wifi_PASS);
            
            hub.sendNotice(F("Настройки wifi обновлены"));
        }
    }

    if (b.Button().icon(ICO_SAVE).noLabel().noTab().size(1, 80).click()) {
        if (!wifi_SSID.isEmpty()) {
            hub.sendAction("wifiUpd");
        } else {
            hub.sendAlert(F("Имя точки доступа не должно быть пустым"));
        }
    }
    
    b.endCol();
    b.endRow();

    b.show();

}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);



    // указать префикс сети, имя устройства и иконку
    hub.config(F("MyDevices"), F("Heater"), F(ICO_APP));

    // подключить билдер
    hub.onBuild(build);

    // запуск!
    hub.begin();
    Serial.println();

#ifdef GH_ESP_BUILD

    // подключение к роутеру
    if (wifiConfig.begin()) {
        Serial.println("Reading wifi settings from FS");
        wifi_SSID = wifiConfig.get("ssid").toString();
        wifi_PASS = wifiConfig.get("pass").toString();
        WiFi.mode(WIFI_STA);
        WiFi.begin(wifi_SSID, wifi_PASS);
        Serial.println();
        while (--wifi_connect_attempts && WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        }
    }
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Non Connecting to WiFi..");
      // режим точки доступа
      WiFi.mode(WIFI_AP);
      WiFi.softAP("Heater");
      Serial.println(WiFi.softAPIP());    // по умолч. 192.168.4.1
    } 
    Serial.println();
    Serial.println(WiFi.localIP());
  
    led.invert(true);
    led.blinkForever(WIFI_BLINK_ON, WiFi.getMode() == WIFI_AP ? WIFI_AP_BLINK : WIFI_STA_BLINK);
#endif

}

void loop() {
    // тикеры модулей
    hub.tick();
    wifiConfig.tick();
    led.tick();

}
