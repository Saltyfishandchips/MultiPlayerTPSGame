# MultiPlayerTPSGame



​		实现了在局域网下以通过Steam在广域网下进行第三人称多人联机的射击游戏，游戏中包含了**手枪、步枪、霰弹枪、榴弹、火箭筒等七类武器**，此外还实现了**速度增益、回血以及护盾**三类增益效果。完成了基本的GamePlay架构后，实现了网络联机下的**延迟补偿策略**。对于有一定Ping值的客户端，**通过服务端倒带来判断客户端是否命中目标**，以获得在延迟环境下更好的游戏体验；在客户端，实现了**客户端预测**算法，在武器射击过程中，**记录发送弹药量的服务端RPC数量**，将接收到的RPC与本地记录进行对比，消除了在延迟环境下，弹药量出现的跳变情况。

-  Demo下载地址：链接：https://pan.baidu.com/s/1FgRDYERYXwyCiQXQwynd6w 提取码：8ett



## 登录界面

![登录界面](C:\Users\Admin\Desktop\TPSGames\登录界面.png)



## 热身大厅

![LobbyMap](C:\Users\Admin\Desktop\TPSGames\LobbyMap.png)



## 热身时间

![热身时间](C:\Users\Admin\Desktop\TPSGames\热身时间.png)



## 团队竞技

![Team](C:\Users\Admin\Desktop\TPSGames\Team.png)



## 个人竞技

![FreeForAll](C:\Users\Admin\Desktop\TPSGames\FreeForAll.png)