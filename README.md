# How to start a compatible MongoDB server with Docker

## Vanilla

```bash
docker run -v $(realpath ./mongodata):/data/db -p 27017:27017 --rm mongo:3.4 mongod --sslMode preferSSL --sslAllowConnectionsWithoutCertificates --sslPEMKeyFile /data/db/combined.pem --sslAllowInvalidCertificates
```

## SCONE

```bash
docker run -e SCONE_MODE=HW -e SCONE_VERSION=1 -v $(realpath ./mongodata):/data/db -p 27017:27017 --rm --device /dev/isgx sconecuratedimages/apps:mongodb-3.4.4-alpine mongod --sslMode preferSSL --sslAllowConnectionsWithoutCertificates --sslPEMKeyFile /data/db/combined.pem --sslAllowInvalidCertificates
```

**Note:** The SCONE version makes the client SEGFAULT often!
