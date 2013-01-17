#include "CompositeFBOQuad.h"


CompositeFBOQuad::CompositeFBOQuad(float locX, float locY, float locZ, float width, float height)
{
	x = locX;
	y = locY;
	z = locZ;
	w = width;
	h = height;

	// Give everything a null value to start with.
	// Notice how we don't yet have an FBO to contend with?
	vertexArray = 0;
	tUnit = 0;
	vboHandles[0] = 0;
	vboHandles[1] = 0;
}

void CompositeFBOQuad::initFBOQuad(char * vertSource, char * fragSource, char *  name)
{
	printf("================================================================\n");
	printf("COMPOSITE FBO QUAD STATUS:\n");
	printf("Generating sampler...\n");
	// Create the sampler id.
	glGenSamplers(1, &sampler);
	
	// Load the vertex shader.
	vert.loadShader(vertSource, GL_VERTEX_SHADER, true);
	vert.printSource();
	
	// Load the fragment shader.
	frag.loadShader(fragSource, GL_FRAGMENT_SHADER, true);
	frag.printSource();

	// Link the two shaders into a shader program.
	shaderProgram.setName(name);
	shaderProgram.createProgram();
	shaderProgram.attachShaderToProgram(&vert);
	shaderProgram.attachShaderToProgram(&frag);
	shaderProgram.linkProgram();

	// Generate the vertex array that we will use for the quad.
	printf("Generating VAO...\n");
	glGenVertexArrays(1, &vertexArray);

	// Generate the vertex buffers that we will use.
	printf("Generating VBOs...\n");
	glGenBuffers(2, vboHandles);

	// Bind to our vertex array so that we can set up its buffers.
	printf("Binding to vertex array...\n");
	glBindVertexArray(vertexArray);

	// Set up buffer 0, which will be our vertex buffer.
	printf("Setting up vertex buffer...\n");
	glBindBuffer(GL_ARRAY_BUFFER, vboHandles[0]);
	// Since we call updateVerts() to actually populate the buffers, we simply declare
	// room for their data here.
	float floatsA[12];
	float floatsB[8];
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(float), floatsA, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Set up buffer 1, which will be our uv coordinate buffer.
	printf("Setting up UV buffer...\n");
	glBindBuffer(GL_ARRAY_BUFFER, vboHandles[1]);
	glBufferData(GL_ARRAY_BUFFER, 8*sizeof(float), floatsB, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Actually store the vertex data.
	updateVerts();
	printf("================================================================\n");
}


CompositeFBOQuad::~CompositeFBOQuad(void)
{
	// Delete the shaders and shader program to clear up GPU memory.
	vert.deleteShader();
	frag.deleteShader();
	shaderProgram.deleteProgram();
	glDeleteVertexArrays(1, &vertexArray);
	glDeleteBuffers(2, vboHandles);
	glDeleteSamplers(1, &sampler);
}

void CompositeFBOQuad::addFBO(GLFrameBuffer * newFBO)
{
	fbos.push_back(newFBO);
	
	int numberOfDigits = 0;
	int fboNumber = fbos.size() - 1;

	if(fboNumber == 0)
	{
		numberOfDigits = 1;
	}
	else
	{
		while(fboNumber != 0) {fboNumber /= 10; numberOfDigits ++;}
	}
	char * name = (char*) malloc(sizeof(char)*(strlen("fboSampler")+numberOfDigits+1));
	sprintf(name, "fboSampler%d\0", fbos.size()-1);
	samplerNames.push_back(name);
	printf("ADDED FBO: %s\n", newFBO->getName());
	printf("\tFBO #: %d\n", fbos.size()-1);
	printf("\tSamplerName: %s\n", name);
	

}

void CompositeFBOQuad::setLocation(float locX, float locY, float locZ)
{

	// Update the coordinates of the quad.
	x = locX;
	y = locY;
	z = locZ;

	// Update the verts with the new location.
	updateVerts();
}

void CompositeFBOQuad::setDimensions(float width, float height)
{
	// Update the dimensions of the quad.
	w = width;
	h = height;

	// Use the updated dimensions to recreate all vertex data.
	updateVerts();
}

void CompositeFBOQuad::updateVerts(void)
{

	// Create an array to store the vertices.
	float * verts = (float *) malloc(sizeof(float)*12);

	// Create all the vertex locations.
	verts[0]  = x;
	verts[1]  = y+h;
	verts[2]  = z;

	verts[3]  = x;
	verts[4]  = y;
	verts[5]  = z;

	verts[6]  = x+w;
	verts[7]  = y+h;
	verts[8]  = z;

	verts[9]  = x+w;
	verts[10] = y;
	verts[11] = z;

	// Create an array to store the vertex UV coordinates.
	float * uv = (float * ) malloc(sizeof(float)*8);

	// Create all the UV coordinates.
	uv[0] = 0.0f;
	uv[1] = 0.0f;

	uv[2] = 0.0f;
	uv[3] = 1.0f;

	uv[4] = 1.0f;
	uv[5] = 0.0f;

	uv[6] = 1.0f;
	uv[7] = 1.0f;

	// If the quad has been initialized, we update our presence in
	// GPU memory by copying over the vertex data.
	if(vboHandles[0] != 0){
		printf("STORING VERT DATA");
		// Bind to our vertex array.
		glBindVertexArray(vertexArray);

		// Copy over the vertex location data.
		glBindBuffer(GL_ARRAY_BUFFER, vboHandles[0]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * sizeof(float), verts);

		// Copy over the vertex UV data.
		glBindBuffer(GL_ARRAY_BUFFER, vboHandles[1]);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 8 * sizeof(float), uv);
	}

	// Now we free the memory we used.
	free(verts);
	free(uv);

}

void CompositeFBOQuad::render(void)
{
	// Start using the shader program.
	shaderProgram.useProgram();

	// Bind the FBO's texture to our sampler.
	for(int i = 0; i < fbos.size(); i ++)
	{
		fbos[i]->getFBOTexture()->bindTexture(i);
		GLuint sampleLoc = 99;
		sampleLoc = glGetUniformLocation(shaderProgram.getProgramID(), samplerNames[i]);
		glUniform1i(sampleLoc, i);
	}
	

	

	// Bind to our vertex array.
	glBindVertexArray(vertexArray);

	// Draw the vertex array, with it all the buffers we set up here.
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// Bind back to the null vertex array.
	glBindVertexArray(0);

	// Stop using the shader program.
	shaderProgram.stopUsingProgram();

}
