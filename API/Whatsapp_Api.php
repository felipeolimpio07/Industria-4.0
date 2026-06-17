<?php
// whatsapp_api.php

// 1. Libera as políticas de CORS para o navegador
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// 2. Responde ao "preflight" (a requisição de segurança do Chrome)
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

header('Content-Type: application/json');

// Garante que a requisição real de disparo seja POST
if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(["status" => "erro", "mensagem" => "Método não permitido. Use POST."]);
    exit;
}

// Recebe a mensagem do Console do Chrome
$input = json_decode(file_get_contents("php://input"), true);
$mensagemAlerta = $input['mensagem'] ?? 'Alerta crítico desconhecido!';

/* * =========================================================
 * INTEGRAÇÃO COM A Z-API
 * ========================================================= */

$apiUrl = "https://api.z-api.io/instances/3F49717D954D0137EDB962ED0328D6D7/token/C9F617AE90FF417B92BAF676/send-text"; 
$numeroDestino = "5541985175707"; 

$payload = json_encode([
    "phone" => $numeroDestino,
    "message" => $mensagemAlerta
]);

$ch = curl_init($apiUrl);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_POST, true);
curl_setopt($ch, CURLOPT_POSTFIELDS, $payload);
curl_setopt($ch, CURLOPT_HTTPHEADER, [
    'Content-Type: application/json'
]);

$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
curl_close($ch);

if ($httpCode >= 200 && $httpCode < 300) {
    echo json_encode(["status" => "sucesso", "mensagem" => "Alerta disparado na Z-API!"]);
} else {
    http_response_code(500);
    echo json_encode(["status" => "erro", "detalhe" => "Falha na comunicação", "resposta_api" => $response]);
}
?>