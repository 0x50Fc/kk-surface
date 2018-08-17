//
//  kk-string.h
//
//  Created by 张海龙 on 2018/02/01
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef _KK_STRING_H
#define _KK_STRING_H

#if defined(__APPLE__)

#include <KKSurface/kk-object.h>

#elif defined(__ANDROID_API__)

#include "kk-object.h"

#endif


namespace kk {

    Boolean CStringHasPrefix(CString string,CString prefix);
    Boolean CStringHasSuffix(CString string,CString suffix);
    Boolean CStringEqual(CString string,CString value);
    size_t CStringLength(CString string);
    void CStringSplit(CString string,CString delim, std::vector<String>& items);
    void CStringSplit(CString string,CString delim, std::set<String>& items);
    String CStringJoin(std::vector<String>& items,CString delim);
    String CStringJoin(std::set<String>& items,CString delim);
    String& CStringTrim(String& string);
    String CStringPathAppend(CString basePath,CString path);
    String CStringPathDeleteLast(CString path);
    String CStringPathDeleteExtension(CString path);
}

#endif
