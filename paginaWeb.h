const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML><html><head>
  <title>Monitor de Temperatura</title>
  <meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <button onclick="sincronizarRelogioESP()">Sincronizar relógio</button>
  <script>
    function sincronizarRelogioESP() {
    const agora = new Date();
    const dia = agora.getDate();
    const mes = agora.getMonth() + 1;
    const ano = agora.getFullYear(); 
    const hora = agora.getHours();
    const minuto = agora.getMinutes();
    const segundo = agora.getSeconds();

    // É fundamental manter esta ordem exata para que o `request->getParam(i)` do C++
    // popule o array dateTime[0..5] de forma correta.

    const queryString = `dia=${dia}&mes=${mes}&ano=${ano}&hora=${hora}&minuto=${minuto}&segundo=${segundo}`;
    
    // Como o HTML geralmente é servido pelo próprio ESP, podemos usar um caminho relativo
    const url = `/get?${queryString}`;

    fetch(url, { method: 'GET' })
      .then(response => {
        if (response.ok) {
          console.log("Tempo sincronizado com o servidor com sucesso!");
        } else {
          console.error("Falha ao sincronizar o tempo.");
        }
      })
      .catch(error => console.error("Erro na requisição:", error));
  }
  </script>
  
  <a href="http://192.168.4.1/dados.txt">Clique aqui para acessar os dados.</a>
  </object>

</body></html>)

)rawliteral";