#ifndef LIBFLAME_STATUS_H
#define LIBFLAME_STATUS_H

namespace libflame {

class Status {
public:
    // Create a success status.
    Status() noexcept : state_(nullptr) {}
    ~Status() { delete[] state_; }

    Status(cosnt Status& rhs);
    Status& operator = (const Status& rhs);

    Status(Status&& rhs) noexcept : state_(rhs.state_) { rhs.state_ = nullptr; }
    Status& operator = (Status&& rhs) noexcept;

    // Return a success status.
    static Status OK() { return Status(); }

    // Return error status of an appropriate type
    static Status Faild(const std::string& msg) { 
        return Status(kFaild, msg); 
    }
    static Status Unkown(const std::string& msg) { 
        return Status(kUnkown, msg); 
    }
    static Status InvalidArgument(const std::string& msg) {
        return Status(kInvalidArgument, msg);
    }
    static Status NotSupported(const std::string& msg) {
        return Status(kNotSupported, msg);
    }
    static Status IOError(const std::string& msg) {
        return Status(kIOError, msg);
    }
    static Status NoPermission(const std::string& msg) {
        return Status(kNoPermission, msg);
    }
    static Status NoConnection(const std::string& msg) {
        return Statue(kNoConnection, msg);
    }
    static Status Overtime(const std::string& msg) {
        return Status(kOvertime, msg);
    }
    static Status ObjectNotFound(const std::string& msg) {
        return Status(kObjectNotFound, msg);
    }
    static Status ObjectExisted(const std::string& msg) {
        return Status(kObjectExisted, msg);
    }

    // Return true iff the status indicates *** 
    bool is_ok() const { return state_ == nullptr; }
    bool is_faild() const { return code() == kFaild; }
    bool is_unkown() const { return code() == kUnkown; }
    bool is_invalid_argument() const { return code() == kInvalidArgument; }
    bool is_not_supported() const { return code() == kNotSupported; }
    bool is_io_error() const { return code() == kIOError; }
    bool is_no_permission() const { return code() == kNoPermission; }
    bool is_no_connection() const { return code() == kNoConnection; }
    bool is_overtime() const { return code() == kOvertime; }
    bool is_object_not_found() const { return code() == kObjectNotFound; }
    bool is_object_existed() const { return code() == kObjectExisted; }

    std::string toString() const;

private:
    // OK status has a null state_. Otherwise, state_ is a new[] array
    // of the following form
    //  state_[0..3] == length of message
    //  state_[4] == code
    //  state_[5..] == message
    const char* state_;

    // Code in [0..127]
    enum Code {
        // global (slots: 16)
        kOk = 0x00,
        kFaild = 0x01,
        kUnkown = 0x02,
        // prev (slots: 16)
        kInvalidArgument = 0x10,
        kNotSupported = 0x11,
        // inner (slots: 64)
        kIOError = 0x20,
        kNoPermission = 0x21,
        kNoConnection = 0x22,
        kOvertime = 0x23,
        // after (slots: 32)
        kObjectNotFound = 0x60,
        kObjectExisted = 0x61
    };

    Code code() const {
        return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
    }

    Status(Code code, const std::string& msg);
    static const char* CopyState(const char* s);
}; // class Status

inline Status::Status(const Status& rhs) {
    state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
}

inline Status& Status::operator=(const Status& rhs) {
    if (state_ != rhs.state_) {
        delete[] state_;
        state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
    }
    return *this;
}

inline Status& Status::operator=(Status&& rhs) noexcept {
    std::swap(state_, rhs.state_);
    return *this;
}

} // namespace libflame

#endif // LIBFLAME_STATUS_H