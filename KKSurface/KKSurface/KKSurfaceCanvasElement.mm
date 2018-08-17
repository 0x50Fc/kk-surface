//
//  KKSurfaceCanvasElement.m
//  KKSurface
//
//  Created by hailong11 on 2018/8/2.
//  Copyright © 2018年 kkmofang.cn. All rights reserved.
//

#import "KKSurfaceCanvasElement.h"
#import "KKSurfaceCGContext.h"

@interface KKSurfaceCanvasElement() {
    BOOL _displaying;
}

@end

@implementation KKSurfaceCanvasElement

+(void) initialize {
    [KKViewContext setDefaultElementClass:[KKSurfaceCanvasElement class] name:@"canvas"];
}


-(void) willInstallSurfaceContext {
    
    [super willInstallSurfaceContext];
    
    __weak KKSurfaceElement * element = self;
    
    [self.context run:^{
        
        KKSurfaceElement * e = element;
        
        if(e) {
            
            KKSurfaceContext * context = e.context;
            
            duk_context * ctx = context.jsContext->jsContext();
            
            [KKSurfaceCGContext openlibs:ctx];
            
            duk_get_global_string(ctx, "surface");
            
            if(duk_is_object(ctx, -1)) {
                
                [KKSurfaceContext JSContextPushSelector:e selector:@selector(setNeedsDisplay) name:"display" ctx:ctx queue:dispatch_get_main_queue()];
                
            }
            
            duk_pop(ctx);
            
        }
        
    }];
    
   
}

-(void) display {
    
    UIView * view = self.view;
    
    if(view == nil) {
        return ;
    }
    
    CGSize size = view.bounds.size;
    CGFloat scale =  view.layer.contentsScale;
    
    __weak KKSurfaceCanvasElement * element = self;
    
    [self.context run:^{
        
        KKSurfaceElement * e = element;
        
        if(e) {
            
            UIGraphicsBeginImageContextWithOptions(size, YES, scale);
            
            CGContextRef CGContext = UIGraphicsGetCurrentContext();
            
            KKSurfaceContext * context = e.context;
            
            duk_context * ctx = context.jsContext->jsContext();
            
            duk_get_global_string(ctx, "surface");
            
            if(duk_is_object(ctx, -1)) {
                
                duk_get_prop_string(ctx, -1, "ondraw");
                
                if(duk_is_function(ctx, -1)) {
                    
                    [KKSurfaceCGContext pushCGContext:CGContext ctx:ctx];
                    
                    if(duk_pcall(ctx, 1) != DUK_EXEC_SUCCESS) {
                        kk::script::Error(ctx, -1);
                    }
                    
                }
                
                duk_pop(ctx);
            }
            
            duk_pop(ctx);
            
            UIImage * image = UIGraphicsGetImageFromCurrentImageContext();
            
            dispatch_async(dispatch_get_main_queue(), ^{
                [element setDisplayImage:image];
            });
            
            UIGraphicsEndImageContext();
        }
        
    }];
    
    _displaying = NO;
}

-(void) setNeedsDisplay {
    
    if(_displaying) {
        return;
    }
    
    __weak KKSurfaceCanvasElement * element = self;
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [element display];
    });
    
}

-(void) setDisplayImage:(UIImage *) image {
   
    UIView * view = self.view;
    
    if(view == nil) {
        return ;
    }
    
    view.layer.contents = (id) image.CGImage;
    
}

-(void) didInstallSurfaceContext {
    [self setNeedsDisplay];
    [super didInstallSurfaceContext];
}

-(void) willUninstallSurfaceContext {
    [super willUninstallSurfaceContext];
}

-(void) didLayouted {
    [super didLayouted];
    [self setNeedsDisplay];
}

@end
