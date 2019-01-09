#ifndef FLAME_INCLUDE_RETCODE_H
#define FLAME_INCLUDE_RETCODE_H

namespace flame {

enum RetCode {
    /**
     * 成功，所有非0的返回都是失败
     */
    RC_SUCCESS = 0,

    /**
     * 默认错误
     */
    RC_FAILD = 1,

    /**
     * 对象已经存在
     */
    RC_OBJ_EXISTED = 2,

    /**
     * 对象不存在
     */
    RC_OBJ_NOT_FOUND = 3,

    /**
     * 错误参数
     */
    RC_WRONG_PARAMETER = 4,

    /**
     * 拒绝操作
     */
    RC_REFUSED = 5,

    /**
     * 内部错误
     */
    RC_INTERNAL_ERROR = 6,

    /**
     * 重复操作
     */
    RC_MULTIPLE_OPERATE = 7
};

} // namespace flame

#endif // !FLAME_INCLUDE_RETCODE_H