#include <Arduino.h>
#include <GyverHub.h>
GyverHub hub;

#include <PairsFile.h>
PairsFile wifiConfig(&GH_FS, "/config/wifi.dat", 3000);

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
    b.Spinner();

    b.show(tab == 1);
    b.Slider();

    b.show(tab == 2);

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

    if (b.Button().icon("f0c7").noLabel().noTab().size(1, 80).click()) {
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
    hub.config(F("MyDevices"), F("Heater"), F("f185"));

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
  
#endif

}

void loop() {
    // 
    hub.tick();
    wifiConfig.tick();

    static gh::Timer tmr(500);
    if (tmr) {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}
