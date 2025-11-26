export module Flow:StatusHelpers;
import Element;
import :StatusDomain;
import :StatusCode;
import :Status;
export [[nodiscard]] inline constexpr Status Ok(StatusDomain domain) noexcept {
     return Status{
        StatusCode{domain, StatusCode::Ok},
        nullptr 
        }; 
    }
export [[nodiscard]] inline constexpr Status Err(StatusDomain domain, StatusCodeValue value, const char* msg = nullptr) noexcept { 
    return Status{ 
        StatusCode{domain,value},
        msg
    };
}

