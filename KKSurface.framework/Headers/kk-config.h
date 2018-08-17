//
//  kk-config.h
//
//  Created by 张海龙 on 2018/02/01
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#ifndef _KK_CONFIG_H
#define _KK_CONFIG_H

#include <sys/time.h>
#include <sys/stat.h>
#include <math.h>
#include <assert.h>

#ifndef MAXFLOAT
#define MAXFLOAT    0x1.fffffep+127f
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#endif
