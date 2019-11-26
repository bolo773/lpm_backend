<?php
$key="01234567890123456789012345678901";
$iv="0123456789012345";
$data = file_get_contents('php://stdin');
$output= openssl_decrypt($data,"AES-256-CBC", $key, OPENSSL_RAW_DATA, $iv);

file_put_contents('phpdec.png',$output);
?>
