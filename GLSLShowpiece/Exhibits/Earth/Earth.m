/*

File: Earth.m

Abstract: Earth Exhibit

 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Computer, Inc. ("Apple") in consideration of your agreement to the
 following terms, and your use, installation, modification or
 redistribution of this Apple software constitutes acceptance of these
 terms.  If you do not agree with these terms, please do not use,
 install, modify or redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software. 
 Neither the name, trademarks, service marks or logos of Apple Computer,
 Inc. may be used to endorse or promote products derived from the Apple
 Software without specific prior written permission from Apple.  Except
 as expressly stated in this notice, no other rights or licenses, express
 or implied, are granted by Apple herein, including but not limited to
 any patent rights that may be infringed by your derivative works or by
 other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

  Copyright (c) 2004-2006 Apple Computer, Inc., All rights reserved.

*/

#import "Earth.h"

@implementation Earth

- (id) init
{
	[super init];
	
	return self;
}

- (void) initLazy
{
	[super initLazy];
	
	/* Setup GLSL */
   {
      NSBundle *bundle;
      NSString *string, *vertex_string, *fragment_string;
		NSBitmapImageRep *bitmapimagerep;
		NSRect rect;

      bundle = [NSBundle bundleForClass: [self class]];

		/* day texture */
		glGenTextures(1, &day_texture);
		string = [bundle pathForResource: @"Day" ofType: @"jpg"];

		bitmapimagerep = LoadImage(string, 1);
		rect = NSMakeRect(0, 0, [bitmapimagerep pixelsWide], [bitmapimagerep pixelsHigh]);

		glBindTexture(GL_TEXTURE_2D, day_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rect.size.width, rect.size.height, 0,
						 (([bitmapimagerep hasAlpha])?(GL_RGBA):(GL_RGB)), GL_UNSIGNED_BYTE, 
						 [bitmapimagerep bitmapData]);

		/* night texture */
		glGenTextures(1, &night_texture);
		string = [bundle pathForResource: @"Night" ofType: @"jpg"];

		bitmapimagerep = LoadImage(string, 1);
		rect = NSMakeRect(0, 0, [bitmapimagerep pixelsWide], [bitmapimagerep pixelsHigh]);

		glBindTexture(GL_TEXTURE_2D, night_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rect.size.width, rect.size.height, 0,
						 (([bitmapimagerep hasAlpha])?(GL_RGBA):(GL_RGB)), GL_UNSIGNED_BYTE, 
						 [bitmapimagerep bitmapData]);

		/* clouds texture */
		glGenTextures(1, &clouds_texture);
		string = [bundle pathForResource: @"Clouds" ofType: @"jpg"];

		bitmapimagerep = LoadImage(string, 1);
		rect = NSMakeRect(0, 0, [bitmapimagerep pixelsWide], [bitmapimagerep pixelsHigh]);

		glBindTexture(GL_TEXTURE_2D, clouds_texture);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rect.size.width, rect.size.height, 0,
						 (([bitmapimagerep hasAlpha])?(GL_RGBA):(GL_RGB)), GL_UNSIGNED_BYTE, 
						 [bitmapimagerep bitmapData]);

		/* Load vertex and fragment shader */
      vertex_string   = [bundle pathForResource: @"Earth" ofType: @"vert"];
      vertex_string   = [NSString stringWithContentsOfFile: vertex_string];
      fragment_string = [bundle pathForResource: @"Earth" ofType: @"frag"];
      fragment_string = [NSString stringWithContentsOfFile: fragment_string];
		if ([self loadVertexShader: vertex_string fragmentShader: fragment_string])
			NSLog(@"Failed to load Earth");
			
		/* Setup uniforms */
		glUseProgramObjectARB(program_object);
		glUniform3fARB(glGetUniformLocationARB(program_object, "LightPosition"), 3.0, 0.0, -3.0);
		glUniform1iARB(glGetUniformLocationARB(program_object, "EarthDay"), 0);
		glUniform1iARB(glGetUniformLocationARB(program_object, "EarthNight"), 1);
		glUniform1iARB(glGetUniformLocationARB(program_object, "EarthCloudGloss"), 2);

	}

}

- (void) dealloc
{
	[super dealloc];
	glDeleteTextures(1, &day_texture);
	glDeleteTextures(1, &night_texture);
	glDeleteTextures(1, &clouds_texture);
}

- (NSString *) name
{
	return @"Earth";
}

- (NSString *) descriptionFilename
{
	NSBundle *bundle;
	NSString *string;
	bundle = [NSBundle bundleForClass: [self class]];
   string = [bundle pathForResource: @"Earth" ofType: @"rtf"];
	
	return string;
}

- (void) renderFrame
{
	[super renderFrame];
	
	glUseProgramObjectARB(program_object);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, day_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, night_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, clouds_texture);
	glActiveTexture(GL_TEXTURE0);
	
	glRotatef(-90, 1.0, 0.0, 0.0);
	
	gluSphere(quadric, 0.5, 30, 30);
	
	glUseProgramObjectARB(NULL);
}

@end
