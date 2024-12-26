# Setting Up DNS Seed Servers for Junkcoin

This guide explains how to set up and maintain DNS seed servers for the Junkcoin network.

## Requirements

- A domain name (e.g., junk-coin.com)
- A Linux server with root access
- Basic knowledge of DNS configuration
- BIND9 DNS server software

## Installation

1. Install BIND9:
```bash
sudo apt-get update
sudo apt-get install bind9 bind9utils bind9-doc
```

2. Configure BIND9:
```bash
sudo nano /etc/bind/named.conf.options
```

Add the following configuration:
```
options {
        directory "/var/cache/bind";
        recursion no;
        allow-transfer { none; };
        dnssec-validation auto;
        auth-nxdomain no;
        listen-on-v6 { any; };
};
```

3. Create zone files:
```bash
sudo nano /etc/bind/named.conf.local
```

Add the following zones:
```
zone "mainnet.junk-coin.com" {
        type master;
        file "/etc/bind/zones/db.mainnet.junk-coin.com";
};

zone "testnet.junk-coin.com" {
        type master;
        file "/etc/bind/zones/db.testnet.junk-coin.com";
};
```

4. Create the zone files:
```bash
sudo mkdir /etc/bind/zones
```

For mainnet:
```bash
sudo nano /etc/bind/zones/db.mainnet.junk-coin.com
```

Add:
```
$TTL    86400
@       IN      SOA     mainnet.junk-coin.com. admin.junk-coin.com. (
                     2023121001         ; Serial
                         604800         ; Refresh
                          86400         ; Retry
                        2419200         ; Expire
                          86400 )       ; Negative Cache TTL
;
@       IN      NS      ns1.junk-coin.com.
@       IN      A       YOUR_SEED_SERVER_IP
```

Repeat for testnet:
```bash
sudo nano /etc/bind/zones/db.testnet.junk-coin.com
```

5. Start BIND9:
```bash
sudo systemctl start bind9
sudo systemctl enable bind9
```

## Running the Seeder

1. Set up a Junkcoin node:
```bash
# For mainnet seeder
./src/junkcoind -daemon -listen -discover

# For testnet seeder
./src/junkcoind -testnet -daemon -listen -discover
```

2. Create a script to update DNS records:
```bash
#!/bin/bash
# update-seeds.sh

# Get connected peers
PEERS=$(./src/junkcoin-cli getpeerinfo | jq -r '.[].addr' | cut -d':' -f1)

# Update zone file
SERIAL=$(date +%Y%m%d%H)
cat > /etc/bind/zones/db.mainnet.junk-coin.com << EOF
\$TTL    86400
@       IN      SOA     mainnet.junk-coin.com. admin.junk-coin.com. (
                     ${SERIAL}         ; Serial
                         604800         ; Refresh
                          86400         ; Retry
                        2419200         ; Expire
                          86400 )       ; Negative Cache TTL
;
@       IN      NS      ns1.junk-coin.com.
EOF

# Add peer records
for PEER in $PEERS; do
    echo "@       IN      A       $PEER" >> /etc/bind/zones/db.mainnet.junk-coin.com
done

# Reload BIND
rndc reload
```

3. Make the script executable and add to crontab:
```bash
chmod +x update-seeds.sh
crontab -e
```

Add:
```
*/30 * * * * /path/to/update-seeds.sh
```

## Maintenance

1. Monitor the logs:
```bash
tail -f /var/log/syslog | grep named
```

2. Check DNS resolution:
```bash
dig @localhost mainnet.junk-coin.com
dig @localhost testnet.junk-coin.com
```

3. Update firewall rules:
```bash
sudo ufw allow 53/udp
sudo ufw allow 53/tcp
```

## Security Considerations

1. Keep BIND9 updated:
```bash
sudo apt-get update
sudo apt-get upgrade bind9
```

2. Configure firewall to only allow necessary ports
3. Monitor for suspicious activity
4. Regularly backup configuration files
5. Use DNSSEC if possible

## Troubleshooting

1. Check BIND9 status:
```bash
sudo systemctl status bind9
```

2. Verify configuration:
```bash
named-checkconf
named-checkzone mainnet.junk-coin.com /etc/bind/zones/db.mainnet.junk-coin.com
```

3. Check logs:
```bash
journalctl -u bind9
```

## Support

For help or to report issues:
- GitHub: [Junkcoin Core repository](https://github.com/Junkcoin-Foundation/junkcoin-core-wallet/tree/dev)
- Email: [contact@junk-coin.com](mailto:contact@junk-coin.com)
