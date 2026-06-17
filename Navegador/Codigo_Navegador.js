// ==UserScript==
// @name         Monitor Tinkercad (Estado Crítico Exato)
// @namespace    http://tampermonkey.net/
// @version      8.0
// @description  Ignora o relatório de tempo e foca no perigo real
// @author       Felipe
// @match        https://www.tinkercad.com/*
// @match        https://tinkercad.com/*
// @include      *://*.tinkercad.com/*
// @all_frames   true
// @grant        GM_xmlhttpRequest
// ==/UserScript==

(function() {
    'use strict';

    console.log("🤖 TAMPERMONKEY (V8) - Focado no Alerta Real!");

    let emCooldown = false;

    // AQUI ESTÁ O SEGREDO: Coloque a frase que o Arduino imprime SOMENTE quando a luz vermelha acende
    const FRASE_DE_PERIGO = "Risco de superaquecimento no motor!";

    setInterval(() => {
        let painelSerial = document.querySelector('.js-code_panel__serial__text');

        if (painelSerial) {
            let texto = painelSerial.innerText || "";

            // Olha para os últimos caracteres para ler apenas as mensagens atuais
            let finalDoLog = texto.slice(-400);

            // Só dispara se achar a FRASE_DE_PERIGO exata e não estiver em tempo de recarga
            if (finalDoLog.includes(FRASE_DE_PERIGO) && !emCooldown) {
                console.log("🚨 ESTADO CRÍTICO ATUAL DETECTADO! Disparando para o WhatsApp...");

                GM_xmlhttpRequest({
                    method: "POST",
                    url: "http://localhost/IA/whatsapp_api.php",
                    headers: { "Content-Type": "application/json" },
                    data: JSON.stringify({
                        mensagem: "🚨 *ALERTA DE TELEMETRIA* 🚨\n\n" + finalDoLog
                    }),
                    onload: function(res) {
                        if(res.status >= 200 && res.status < 300) {
                            console.log(`✅ Sucesso! Mensagem disparada.`);
                        }
                    }
                });

                // Trava o sistema por 20 segundos para não mandar mensagens repetidas
                emCooldown = true;
                setTimeout(() => {
                    emCooldown = false;
                    console.log("🟢 Robô pronto para novo disparo.");
                }, 20000);
            }
        }
    }, 2000);

})();
