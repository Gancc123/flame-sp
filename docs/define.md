概念定义
----------------

* __Flame__:
分布式块存储系统

* __Flame Service__:
Flame块存储系统的服务层，提供NVMe over Fabrics 和 iSCSI 的Target服务，不持久存储数据

* __Flame Store__:
Flame块存储系统的存储层，提供基于块数据持久化存储，基于Libflame提供块存储服务

* __Volume__:
逻辑卷或虚拟块设备，Libflame对外提供的基本操作单元

* __Volume Group__:
逻辑卷组，便于多用户管理，当前同组的Volume没有任何条件约束

* __Chunk__:
Flame内部将Volume切分成多个Chunk，Flame以Chunk为数据管理单位

* __CSD__:
(Chunk Store Daemon/Device) 数据存储节点提供以Chunk为单位的数据访问服务

* __MGR__:
(Manager) 管理节点

* __Libflame__:
Flame Store对外提供的服务接口库


常用简称
-------

* __clt__:
Cluster

* __vg__:
Volumn Group

* __vol__:
Volume

* __chk__:
Chunk

* __hlt__:
Health