#!/bin/bash

if ls -d /usr/include/cryptopp &>/dev/null || \
   ls -d /usr/local/include/cryptopp &>/dev/null; then
  echo "cryptopp";
  exit 0;
fi

if ls -d /usr/include/crypto++ &>/dev/null || \
   ls -d /usr/local/include/crypto++ &>/dev/null; then
  echo "crypto++";
  exit 0;
fi

