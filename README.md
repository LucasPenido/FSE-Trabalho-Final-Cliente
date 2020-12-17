# Trabalho Final - FSE - Sistema Sistribuído de Automação Residencial 
aluno: Lucas Penido Antunes  
matrícula: 16/0013143

Repositório com a solução do trabalho final.

## Projeto

O objetivo deste trabalho é criar um sistema distribuído de automação residencial utilizando como sistema computacional central uma placa Raspberry Pi 4 e como controladores distribuídos placas ESP32, interconectados via Wifi através do protocolo MQTT.

Para uma maior descrição do projeto visitar este [GitLab](https://gitlab.com/fse_fga/projetos/trabalho-final).

### Configurando o programa

Para configurar o Wifi execute o comando:  

`idf.py menuconfig` 

Após o menu abrir selecione a opção "Configuração do Wifi" e configure o SSID e a senha. Depois na opção "Configuração de Matrícula" configure a matrícula do aluno.  

Após todas as configurações serem feitas rode o seguinte comando para dar o build no projeto.

`idf.py build`

Após feita a build do projeto, rode o seguinte comando para enviar o progama para a ESP32 e monitorá-lo. Trocando (PORT) para a porta que a ESP32 está conectada:

`idf.py -p (PORT) flash monitor`
