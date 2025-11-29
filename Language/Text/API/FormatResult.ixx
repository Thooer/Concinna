export module Lang.Text:FormatResult;
import Lang.Element;

export enum class FormatStatus : UInt8 { Ok, Truncated, InvalidSpec };
export struct FormatResult { FormatStatus status; USize written; };
