# **Tracked Direction**  
### *Joystick Data Logger for IoT*  

**Descrição:**  
Projeto desenvolvido para a placa **Bitdoglab/RP2040** que monitora a posição de um joystick, converte as coordenadas em direções cardinais (Norte, Sul, Leste, Oeste, etc.) e envia os dados para um servidor local ou na nuvem via Wi-Fi. Inclui também a leitura de botões e um sensor adicional (desafio extra).  

---

## **Funcionalidades**  
✅ **Leitura do Joystick:**  
- Captura os valores analógicos dos eixos **X** e **Y**.  
- Mapeia as coordenadas para direções cardinais (ex: `X=255, Y=0 → "Leste"`).  

✅ **Monitoramento de Botões:**  
- Envia o status dos botões da placa a cada **1 segundo**.  

---

## **Tecnologias Utilizadas**  
- **Hardware:**  
  - Placa Bitdoglab com RP2040.  
  - Joystick analógico.
- **Software:**  
  - Linguagem: **C (bare metal)**.  
  - Protocolo: **MQTT** (para nuvem) ou **HTTP** (para servidor local).  
  - Ferramentas: **Wokwi** (simulação), **AWS IoT Core/Firebase** (nuvem).  

---

**Nota:** Este projeto foi desenvolvido como parte da Unidade 2 da disciplina *"Aplicações com Comunicação Sem Fio para IoT"*.  

--- 

### **Capturas de Tela (Opcional)**  
*(Adicione imagens do Wokwi/Bitdoglab em ação aqui)*   

--- 

**Atualizado em:** `08/04/2025`  

---

## **Autor:**
Desenvolvido por [Rafhael Gaspar da silva](https://github.com/Rafhael0069).