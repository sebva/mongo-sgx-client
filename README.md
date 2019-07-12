# Port of MongoDB C client for SGX

## License

```
A-SKY: Anonymous and Confidential File Sharing over Untrusted Clouds
Copyright (C) 2018-2019 Rafael Pires, Sébastien Vaucher

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

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
