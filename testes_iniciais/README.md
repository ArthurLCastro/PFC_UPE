# Testes Iniciais

## **Orientações para compilação**:
Para que firmwares que fazem uso de WiFi possam ser compilados corretamente, é necessário definir o *SSID* e *senha* da rede que o ESP32 irá se conectar.

Por questões de segurança essas definições são feitas separadas do código fonte. Dessa forma, é necessário que ao lado do firmware (que utilizar WiFi) seja criado um arquivo chamado **env.h** com a seguinte estrutura:

```cpp
// WiFi Config
#define ENV_WIFI_SSID        "SSID da rede WiFi aqui"
#define ENV_WIFI_PASSWORD    "Senha da rede WiFi aqui"
```