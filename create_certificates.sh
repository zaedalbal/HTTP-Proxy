#!/bin/bash
set -e

if [ $# -lt 1 ]; then
    echo "Error: you must specify a directory to store the certificates."
    echo "Usage: $0 <directory>"
    exit 1
fi

CERT_DIR="$1"
KEY_NAME="server.key"
CERT_NAME="server.crt"
DAYS_VALID=365  # 1 год
KEY_SIZE=4096

mkdir -p "$CERT_DIR"

echo "Generating private key (${KEY_SIZE} bits)..."
openssl genpkey -algorithm RSA -out "$CERT_DIR/$KEY_NAME" -pkeyopt rsa_keygen_bits:$KEY_SIZE

echo "Creating certificate signing request (CSR)..."
openssl req -new -key "$CERT_DIR/$KEY_NAME" -out "$CERT_DIR/server.csr" -subj "/C=RU/ST=Moscow/L=Moscow/O=MyServer/OU=IT/CN=localhost"

echo "Generating self-signed certificate (valid for $DAYS_VALID days)..."
openssl x509 -req -days $DAYS_VALID -in "$CERT_DIR/server.csr" -signkey "$CERT_DIR/$KEY_NAME" -out "$CERT_DIR/$CERT_NAME"

echo "Cleaning up temporary files..."
rm "$CERT_DIR/server.csr"

echo "Certificate and key successfully created in $CERT_DIR:"
echo "Certificate: $CERT_NAME"