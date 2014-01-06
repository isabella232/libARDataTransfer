//
//  ViewController.m
//  TestARDataTransfer
//
//  Created by David on 22/11/2013.
//  Copyright (c) 2013 Parrot SA. All rights reserved.
//

#import "ViewController.h"

extern void test_manager(const char *tmp, int opt);

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    NSString *tmp = NSTemporaryDirectory();
    char path[512];
    int opt = 0;
    
    [tmp getFileSystemRepresentation:path maxLength:sizeof(path)];
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    const char* doc = [[paths lastObject] cString];

    opt = 0;
    test_manager(doc, opt);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
