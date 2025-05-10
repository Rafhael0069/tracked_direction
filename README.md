# **Tracked Direction**

### *Joystick Controller with LED Matrix & IoT Data Sync*

**Descrição:**
Projeto desenvolvido para a placa **Bitdoglab/RP2040** que realiza a leitura da posição de um **joystick analógico**, converte as coordenadas em direções e controla uma **matriz de LEDs RGB 5x5** com feedback visual em tempo real. Além disso, envia os dados coletados para um **servidor remoto** via **Wi-Fi**, permitindo integração com aplicações IoT locais ou na nuvem.

Inclui também a leitura do botão do joystick e transmissão periódica das informações para monitoramento externo.

---

## **Funcionalidades**

✅ **Leitura do Joystick:**

* Captura valores analógicos dos eixos **X** e **Y**.
* Normaliza os valores para a faixa de **-10 a 10**, facilitando a interpretação dos movimentos.
* Detecta o pressionamento do botão central do joystick.

✅ **Controle da Matriz de LEDs (5x5):**

* Exibe visualmente a direção movida pelo joystick com animações coloridas.
* Permite ajuste do brilho global dos LEDs para otimização visual e energética.

✅ **Envio dos Dados via Wi-Fi:**

* Transmite os valores do joystick e o status do botão para um servidor remoto a cada **1 segundo**.
* Compatível com servidores locais (HTTP) ou nuvem (ex: AWS, Firebase).

---

## **Tecnologias Utilizadas**

* **Hardware:**

  * Placa Bitdoglab com RP2040 (baseada no Raspberry Pi Pico W).
  * Joystick analógico com botão integrado.
  * Matriz de LEDs RGB WS2812B (5x5).

* **Software:**

  * Linguagem: **C (bare metal)**.
  * Comunicação: **HTTP** (atualmente implementado) e compatível para expansão com **MQTT**.

---

**Nota:** Este projeto foi desenvolvido como parte da *Unidade 2* da disciplina *"Aplicações com Comunicação Sem Fio para IoT"*, aplicando conceitos reais de sistemas embarcados conectados. 

--- 

**Atualizado em:** `10/05/2025`

---

## **Autor:**
Desenvolvido por [Rafhael Gaspar da silva](https://github.com/Rafhael0069).