#ifndef MODEL_H
#define MODEL_H
#include "Shader.h"
#include <vector>
#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
using namespace std;

class Model
{
public:
	/*  Functions   */
	Model(char *path);
	void Draw(Shader shader);
private:
	/*  Model Data  */
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	
	/*  Functions   */
	void loadModel(string path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		string typeName);
};

#endif // !MODEL_H

