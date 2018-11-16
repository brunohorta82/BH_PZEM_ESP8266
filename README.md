# BH PZEM



**Conteúdo **   
1. [Intro](#id1)
2. [Hardware](#id2)
3. [Firmware](https://github.com/brunohorta82/BH-Easy-Iot)
4. [Hack de sentido da corrente](#id4)
5. [Exemplos](#id5)
   - [BH PZEM-004 Wi-Fi](#id6)
5. [Donativos](#id7)


## Intro <a name="id1"></a>


**BH Pzem** é um dispositivo baseado em ESP8266 e PZEM-004 que mede energia e / ou temperatura e enviaos dados a um broker MQTT e / ou Emoncms. O BH Pzem é modular, o que significa que podemos começar com um simples módulo de energia até um sistema de monitorização de produção energética, juntamente com detecção de temperatura (até 10 sensores de temperatura digital). Podemos ainda adicionar um diplay Oled e relés.
Os dados são enviados via Wi-Fi

**Painel de Controlo:**
![BHPZem](https://github.com/brunohorta82/BH_PZEM_ESP8266/blob/master/images/current_panel.png)


**Medição de Energia:**
  - Baseado no PZEM-004
  - Leituras muito rigorosas 
  - Possibilizade de detecão de sentido de energia 
  - Voltagem, Ampereragem, Potência, Energia

**Medição de Temperatura:**
  - Baseado no sendor 1-wire DS18B20
  - Permite adicionar até 10 sensores digitais DS18B20
  - Endereços dos sensores geridos na interface Web

**Relé :**
  - Controlado via MQTT

**ESP8266**:
  - Facilmente configurado na Interface Web
  - Envio dos dados em JSON via MQTT
  - Envio dos dados para um servidor Emoncms local/remoto
  
## Hardware <a name="id2"></a>

![BHPZem](https://raw.githubusercontent.com/brunohorta82/BH_PZEM_ESP8266/master/images/PIN_OUT.png)



Qtd | Item | Imagem
:---: | :---: | ---:
1 | PZEM-004 | `...`
1 | ESP-12E | `...`
1 | PCB BH-PZEM Wi-Fi | `...`
1 | Fonte HLK-PM01 | `...`
1 | Regulador de Tensão AMS1117 3.3v | `...`
1 | Condensador Cerâmico 100nF SMD | `...`
5 | Resistência 10K 0603 | `...`
1 | Resistência 4.7K 0603 | `...`
1 | Resistência 1K 0603 | `...`
1 | Optocopolador PC | `...`
2 | Botão 3*4*2 MM SMD | `...`
1 | Fusível T2A 2A 250 | `...`
1 | Sensor temperatua DS18B20 | `...`
1 | Terminal de Parafuso 2P 5.0mm  | `...`
2 | Terminal 2P 2.54mm  | `...`
1 | Conector de 9 Pinos Fêmea 2.54mm | `...`
1 | Conector de 8 Pinos Fêmea 2.54mm | `...`
1 | Conector de 4 Pinos Macho 2.54mm | `...`
1 | Conector de 7 Pinos Macho 2.54mm | `...`


## Software <a name="id3"></a>

- Arduino Ide
- Editar o ficheiro config.h com o setup inicial
- Upload do Firmware para o ESP8266
- Aceder ao AP `SSID:BH_PZEM_NODE_ID` e configurar a Rede Wi-Fi
- Navegar até  `http://<bh-pzem name>.local` ou `http://<bh-pzem IP address>`
- Verificar o sentido da corrente caso tenham feito o Hack para a deteção
- Configurar o `MQTT broker` e/ou a conta do `Emoncms` 
- E tá feito, agora é só curtir :) 


## Hack para deteção de sentido de corrente <a name="id4"></a>

![](https://lh3.googleusercontent.com/7oKvPvdqY3lP6zBprZ33XxJ7tKCSPjcxN2w-OqwHl_1SzNu80wvdnXQUsuom1qUd44zljcSAKoW6R_QD6Fw6dyNdrQpZGyCAt7fK76CRAaeEPu2qT8z-xMeqMLAlgMNtwaDBF7miqPXhsgoQ-rWaterlj8KB61w-i8nbBX3sqG_NviRIJFRPZu4y5Mk5QQHA83q366xY_NipzpD8CfSDKwin57H3Htds_hMZ-07q1bP9mxqiQBRQqcq6nFyu7SyfzQPI7FPg7txX6qM7KTZdOb53coQP14PXP9ZpZgZd6tcLpuRveteLxfdhUHiUXz74R1f_4OeA_25O--yq3BcZT_W89wb5o-ZRAX4VKxyWw7SOmyMhGib1QK7HJxYOTOWC5yf9IasuEWV_vbtsjChd_gD7Al2RD4akmTF-P8RQy6oLgGEKNINEdbHCTc4NhU0PtUVe3b90hhll2i0vlXHoQQ_Dcv1H4UQkiork0te3gWIpN9IEScdoKep3nyKpx4NwDmuup3uilXumtjUWld5qL1ifNlgKCKj39EcPMyswWAbB7pGrEsRqt0L_xvEW9gpEuPkjH6MUEL-biGVslIl6hkCvgLHb1iIcVd2EOjmNMurGXLUkB2KS3h3Vw4LIQRWzvFUrmJfLbP9hZKorvSnoDPO2MfHhwYYeXgx2m2J11w=w800-h402-no)


## Exemplos de BH-PZEM <a name="id5"></a>

### BH PZEM-004 Wi-Fi <a name="id6"></a>
![](https://github.com/brunohorta82/BH_PZEM_ESP8266/blob/master/images/IMG_4417.JPG?raw=true)
![](https://github.com/brunohorta82/BH_PZEM_ESP8266/blob/master/images/IMG_4418.JPG?raw=true)
![](https://github.com/brunohorta82/BH_PZEM_ESP8266/blob/master/images/IMG_4419.JPG?raw=true)


## Caixas e Adaptadores
 - [BHPZEM Suporte DIN Powered by Bruno Horta](https://cad.onshape.com/documents/db83fdb5f2287efd0e4b6ae1/w/3f678a7db5145870a715b4d4/e/e8a96d2384c3348520138283)

## Donativos<a name="id7"></a>

Se gostas-te do projeto podes fazer o teu donativo :).

[![Donativo](https://img.shields.io/badge/Donate-PayPal-green.svg)](https://www.paypal.me/bhonofre)
