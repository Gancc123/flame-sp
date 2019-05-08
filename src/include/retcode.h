#ifndef FLAME_INCLUDE_RETCODE_H
#define FLAME_INCLUDE_RETCODE_H

namespace flame {

enum RetCode {
    /**
     * 成功，所有非0的返回都是失败
     */
    RC_SUCCESS = 0,
#define RC_STR_SUCCESS "success"

    /**
     * 默认错误
     */
    RC_FAILD = 1,
#define RC_STR_FAILED "failed"

    /**
     * 对象已经存在
     */
    RC_OBJ_EXISTED = 2,
#define RC_STR_OBJ_EXISTED "object existed"

    /**
     * 对象不存在
     */
    RC_OBJ_NOT_FOUND = 3,
#define RC_STR_OBJ_NOT_FOUND "object not found"

    /**
     * 错误参数
     */
    RC_WRONG_PARAMETER = 4,
#define RC_STR_WRONG_PARAMETER "wrong parameter"

    /**
     * 拒绝操作
     */
    RC_REFUSED = 5,
#define RC_STR_REFUSED "operate refused"

    /**
     * 内部错误
     */
    RC_INTERNAL_ERROR = 6,
#define RC_STR_INTERNAL_ERROR "internal error"

    /**
     * 重复操作：指无意义的重复
     */
    RC_MULTIPLE_OPERATE = 7
#define RC_STR_MULTIPLE_OPERATE "multiple operate"
};

} // namespace flame

#endif // !FLAME_INCLUDE_RETCODE_H