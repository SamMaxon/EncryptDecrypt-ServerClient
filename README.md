# EncryptDecrypt-ServerClient
Encryption and Decryption Server/Client for sending messages


Five small programs that encrypt and decrypt information using a one-time pad-like system. These programs combine multi-processing code with socket-based inter-process communication. Accessible from the command line using standard Unix features like input/output redirection, and job control. 

Syntax: 
enc_server listening_port
dec_server listening_port
enc_client plaintext key port
dec_client plaintext key port
keygen keylength

