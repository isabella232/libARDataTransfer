//
//  ViewController.m
//  ARTest
//
//  Created by David on 22/11/2013.
//  Copyright (c) 2013 Parrot SA. All rights reserved.
//

#import "ViewController.h"


extern void test_ftp_curl(const char *tmp);
extern void test_ftp_connection(const char *tmp);
extern void test_manager(const char *tmp, const char *opt);

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    NSString *tmp = NSTemporaryDirectory();
    char path[512];
    [tmp getFileSystemRepresentation:path maxLength:sizeof(path)];
    
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    const char* doc = [[paths lastObject] cString];
    
    test_manager(doc, 0);
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
