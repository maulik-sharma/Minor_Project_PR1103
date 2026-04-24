#!/bin/bash
# Sets up Apache httpd to serve the project's content/ dir on port 8081.
# Run once with sudo:  bash test/setup_apache.sh

set -e
CONTENT="$(realpath "$(dirname "$0")/../content")"

sudo tee /etc/apache2/sites-available/benchmark.conf > /dev/null <<EOF
Listen 8081
<VirtualHost *:8081>
    DocumentRoot "$CONTENT"
    <Directory "$CONTENT">
        Options Indexes FollowSymLinks
        AllowOverride None
        Require all granted
    </Directory>
    ErrorLog /dev/null
    CustomLog /dev/null combined
</VirtualHost>
EOF

sudo a2ensite benchmark
sudo a2dissite 000-default 2>/dev/null || true
# www-data must be able to traverse the home directory
sudo usermod -aG "$(id -gn)" www-data
sudo systemctl stop apache2 && sudo systemctl start apache2

echo "Apache httpd is now serving $CONTENT on port 8081"
curl -s -o /dev/null -w "Health check: HTTP %{http_code}\n" http://localhost:8081/index.html
