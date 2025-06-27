import base64
import hmac
from urllib.parse import quote
import time

# 中国移动官方文档给出的核心秘钥计算算法
def token(id,access_key):  
    version = '2018-10-31'
    res = 'products/%s' % id  # 通过产品ID访问产品API
    # 用户自定义token过期时间
    et = str(int(time.time()) + 63072000) # 设置为2年有效时间
    # 签名方法，支持md5、sha1、sha256
    method = 'sha1'
    # 对access_key进行decode
    key = base64.b64decode(access_key)
    # 计算sign
    org = et + '\n' + method + '\n' + res + '\n' + version
    sign_b = hmac.new(key=key, msg=org.encode(), digestmod=method)
    sign = base64.b64encode(sign_b.digest()).decode()
    # value 部分进行url编码，method/res/version值较为简单无需编码
    sign = quote(sign, safe='')
    res = quote(res, safe='')
    # token参数拼接
    token = 'version=%s&res=%s&et=%s&method=%s&sign=%s' % (version, res, et, method, sign)
    return token

username    = "9L83yUyQ3a"                                           # 产品ID
accesskey   = "e5sv12lhq3d/Xfd598cfkckFUWPrjTEh5aNLr/2RLys="         # accessKey
password = token(username, accesskey)
print(password)

# 等待用户按下 Enter 键再退出
input("按 Enter 键退出程序...")