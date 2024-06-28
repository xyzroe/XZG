# WireGuard VPN

## WireGuard Interface Settings

### IP address of the local interface

This is the local peer IP address of the WireGuard interface. It should match `AllowedIPs` section in server config.

### Local Subnet

The subnet for the local peer IP address

### Local port

Not sure what this is but it works with `33333`

### Local Gateway

Not sure what this is but it works with `0.0.0.0`

### Private key of the local interface

The private key of the local peer. Note the public counterpart for registration on the server.

### Address of the endpoint peer

Address of the WG server

### Public key of the endpoint peer

The public key of the WG server

### Port of the endpoint peer

Port of the WG server, normally `51820`

### Allowed IPs

A comma separated list of the IPs allowed to connect to this device via WG

### Allowed Subnet

A comma separated list of the subnets allowed to connect to this device via WG

### Make default

### Pre-shared key

## Example

!!! warning "Demo data" Please generate new keys and use appropriate IP ranges for use in real life scenarios!

### Server config

```
[Interface]
Address = 10.100.0.1/24
ListenPort = 51820
PrivateKey = Server+PrivateKey=
#PublicKey = Server+PublicKey=

[Peer]
PublicKey = Peer+PublicKey=
AllowedIPs = 10.100.0.2/32
```
### CLient config

* IP address of the local interface: 10.100.0.2
* Local Subnet: 255.255.255.0
* Local port: 33333
* Local Gateway: 0.0.0.0
* Private key of the local interface: Peer+PrivateKey=
* Address of the endpoint peer: [SERVER_IP]
* Public key of the endpoint peer: Server+PublicKey=
* Port of the endpoint peer: 51820
* Allowed IPs: 0.0.0.0
* Allowed Subnet: 0.0.0.0
