# How to start a compatible MongoDB server with Docker

## Vanilla

```bash
docker run -v $(realpath ./mongodata):/data/db -p 27017:27017 --rm mongo:3.4 mongod --sslMode preferSSL --sslAllowConnectionsWithoutCertificates --sslPEMKeyFile /data/db/combined.pem --sslAllowInvalidCertificates
```

## SCONE

```bash
docker run -e SCONE_MODE=HW -e SCONE_VERSION=1 -v $(realpath ./mongodata):/data/db -p 27017:27017 --rm --device /dev/isgx sconecuratedimages/apps:mongodb-3.4.4-alpine mongod --sslMode preferSSL --sslAllowConnectionsWithoutCertificates --sslPEMKeyFile /data/db/combined.pem --sslAllowInvalidCertificates
```

## SCONE with Replica Set of 2 machines + 1 arbiter

### On the 2 SGX hosts (adapt `--bind_ip`):

```bash
mkdir -p mongostatic
openssl req -x509 -newkey rsa:2048 -keyout mongostatic/key.pem -out mongostatic/cert.pem -days 365 -nodes
cat mongostatic/key.pem mongostatic/cert.pem > mongostatic/combined.pem
docker run --network host -d -e SCONE_MODE=HW -e SCONE_VERSION=1 -v $(realpath ./mongodata):/data/db -v $(realpath ./mongostatic):/data/static --device /dev/isgx sconecuratedimages/apps:mongodb-3.4.4-alpine mongod --sslMode preferSSL --sslAllowConnectionsWithoutCertificates --sslPEMKeyFile /data/static/combined.pem --sslAllowInvalidCertificates --replSet rs0 --bind_ip sgx-2.maas
```

### On the arbiter (non-SGX OK)

```bash
mkdir -p mongostatic
openssl req -x509 -newkey rsa:2048 -keyout mongostatic/key.pem -out mongostatic/cert.pem -days 365 -nodes
cat mongostatic/key.pem mongostatic/cert.pem > mongostatic/combined.pem
docker run --network host -d -v $(realpath ./mongodata):/data/db -v $(realpath ./mongostatic):/data/static mongo:3.4 mongod --sslMode preferSSL --sslAllowConnectionsWithoutCertificates --sslPEMKeyFile /data/static/combined.pem --sslAllowInvalidCertificates --replSet rs0 --bind_ip hoernli-9.maas
```

### Join the replica

1. Open a Mongo shell
2. Execute the following command (adapt the hosts):
```javascript
rs.initiate({
    _id: "rs0",
    members: [
        { _id:0, host:"sgx-2.maas:27017" },
        { _id:1, host:"sgx-3.maas:27017" },
        { _id:2, host:"hoernli-9.maas:27017", arbiterOnly: true }
    ]
})
```
3. Use the following connection URL in the client:
```
mongodb://sgx-2.maas:27017,sgx-3.maas:27017/?replicaSet=rs0&ssl=true&sslAllowInvalidCertificates=true&sslAllowInvalidHostnames=true
```

# Deploying to Kubernetes

1. Configure your Docker login on Kubernetes for access to private SCONE repository: https://kubernetes.io/docs/tasks/configure-pod-container/pull-image-private-registry/
```
kubectl create secret docker-registry regcred --docker-server=<your-registry-server> --docker-username=<your-name> --docker-password=<your-pword> --docker-email=<your-email>
```
2. Create a secret for the certificate file of Mongo:
```
openssl req -newkey rsa:2048 -new -x509 -days 365 -nodes -out mongodb-cert.crt -keyout mongodb-cert.key
cat mongodb-cert.key mongodb-cert.crt > combined.pem
kubectl create secret generic mongo-certificate --from-file=combined.pem=combined.pem
```
3. Deploy MongoDB
```
kubectl apply -f mongodb-service.yml
```
