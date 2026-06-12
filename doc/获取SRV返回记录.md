# Linux 下

The user is saying that mongos2 is configured for overseas (境外) but a domestic machine is still resolving to it. This means the DNS resolution is not matching the expected "境内" line.

Let me think about what could cause this. The most likely reasons:

1. The "境内" SRV record might not be added correctly - maybe the user added "境内" but the default record is still the one being returned
2. The client's DNS server might be abroad, making DNSPod think it's "境外"
3. The user's machine might be using a DNS server that DNSPod identifies as "境外"

Wait, the user said earlier they only had "默认" and "境外" records, no "境内". Let me re-read the conversation...

From earlier:
- User added: 默认 → mongos4, 境外 → mongos1/mongos2
- They were going to add 境内 → mongos-cn1

But now the error shows `mongos2.mongo.ericyonng.com:27020` - so it's connecting to an overseas node from a domestic machine.

Possible reasons:
1. The "境内" record hasn't been added yet (most likely)
2. The DNS server `183.60.83.19` (Tencent Cloud) might be classified as "境外" by DNSPod - unlikely but possible
3. The user added 境内 records but TTL hasn't expired
4. The "境内" record was added but to the wrong subdomain

The error says `mongos2` which maps to the overseas line. If the "境内" record was added successfully, the client should NOT get mongos2.

Let me ask the user what their current DNS setup looks like and what `dig` shows now.
DNS 智能解析匹配到"境外"去了，几个可能原因：

---

### 最可能：境内记录压根没加或者线路不对

你之前只加了 `默认` 和 `境外`，没加 `境内`。先确认现在控制台里有没有"境内"线路的记录：

```bash
# 同时查权威服务器，看有没有境内记录
dig SRV _mongodb._tcp.mongoscluster.ericyonng.com @f1g1ns1.dnspod.net
```

如果权威服务器也只返回 mongos2 / mongos4，说明控制台配置就是缺的。

---

### 如果确认已加了境内记录

可能是你的 DNS 出口 IP 被 DNSPod 误判为"境外"。验证：

```bash
# 看你的 DNS 请求源 IP 被 DNSPod 归属到什么线路
# 用腾讯云的这个工具
curl https://dnspod.cloud.tencent.com

# 或者查你的 DNS 出口 IP
nslookup myip.opendns.com resolver1.opendns.com
```

你之前 dig 的 DNS 服务器是 `183.60.83.19`（腾讯云），这个大概率被 DNSPod 判为"境内"。但如果你的机器配置了比如 Google DNS `8.8.8.8` 或其他海外 DNS 作为 resolver，DNSPod 收到查询请求时看到的是该 DNS 服务器的 IP（海外），就会返回"境外"线路的结果。

检查：

```bash
cat /etc/resolv.conf
# 如果 nameserver 是 8.8.8.8 或 1.1.1.1 之类的海外 DNS，就是这个原因
```

---

### 快速定位

```bash
# 从出问题的机器直接 dig 看解析到谁
dig SRV _mongodb._tcp.mongoscluster.ericyonng.com +short
```

返回结果后告诉我，以及你 `/etc/resolv.conf` 里的 nameserver 是什么。



# Windows下

The user is on Windows and can't use Linux commands like dig, curl, etc. Let me give them Windows equivalents.

The key issue is: their SRV record somehow resolves to mongos2 (which is marked as "境外" line in DNSPod), even though they expected it to resolve to a domestic node. 

On Windows:
- `dig` → `nslookup -type=SRV` or `Resolve-DnsName`
- `cat /etc/resolv.conf` → `ipconfig /all` or `Get-DnsClientServerAddress`
- `curl` → same, curl works on Windows too

Let me give them the Windows equivalents.
用 Windows 自带工具就行：

---

### 查 SRV 解析结果

```powershell
# PowerShell
Resolve-DnsName -Type SRV _mongodb._tcp.mongoscluster.ericyonng.com

# 或者 cmd
nslookup -type=SRV _mongodb._tcp.mongoscluster.ericyonng.com
```

---

### 查看当前 DNS 服务器

```powershell
# PowerShell
Get-DnsClientServerAddress -AddressFamily IPv4

# 或者 cmd
ipconfig /all | findstr "DNS"
```

---

### 验证权威服务器有几条记录

```powershell
nslookup -type=SRV _mongodb._tcp.mongoscluster.ericyonng.com f1g1ns1.dnspod.net
```

---

先跑 `Resolve-DnsName`，看返回了几条结果、分别指向哪个 mongos。同时看下 DNS 服务器地址是不是配了海外的（比如 `8.8.8.8`），是的话就是原因——DNSPod 看到 Google DNS 的 IP 来源，直接判定"境外"了。