//
//  KKSurfaceElement.h
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import <KKView/KKView.h>
#import <KKSurface/KKSurfaceContext.h>


@interface KKSurfaceElement : KKViewElement

@property(nonatomic,retain,readonly) KKSurfaceContext * context;

-(void) willInstallSurfaceContext;

-(void) didInstallSurfaceContext;

-(void) willUninstallSurfaceContext;

@end
