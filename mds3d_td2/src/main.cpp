#include "viewer.h"
#include "mesh.h"

#include <filesystem/resolver.h>

void renderBlock(Scene* scene, ImageBlock& block)
{
    const Camera *camera = scene->camera();

    float tanfovy2 = tan(camera->fovY()*0.5);
    Vector3f camX = camera->right() * tanfovy2 * camera->nearDist() * float(camera->vpWidth())/float(camera->vpHeight());
    Vector3f camY = camera->up() * tanfovy2 * camera->nearDist();
    Vector3f camF = camera->direction() * camera->nearDist();

    const Integrator* integrator = scene->integrator();
    integrator->preprocess(scene);

    /* Clear the block contents */
    block.clear();

    Vector2i offset = block.getOffset();
    Vector2i size  = block.getSize();

    /* For each pixel and pixel sample */
    for (int y=0; y<size.y(); ++y) {
        for (int x=0; x<size.x(); ++x) {
            Color3f color(0.,0.,0.);
            // compute the primary ray parameters
            Ray ray;
            ray.origin = camera->position();
            ray.direction = (camF + camX * (2.0*float(x + 0.5f + offset[0])/float(camera->vpWidth())  - 1.)
                    - camY * (2.0*float(y + 0.5f + offset[1])/float(camera->vpHeight()) - 1.)).normalized();

            // raytrace the ray
            color += integrator->Li(scene,ray);

            block.put(Vector2f(x+offset[0], y+offset[1]),color);
        }
    }
}

void render(Scene* scene, ImageBlock* result, std::string outputName, bool* done)
{
    if(!scene)
        return;

    clock_t t = clock();
    Mesh::ms_itersection_count = 0;

    Vector2i outputSize = scene->camera()->outputSize();

    /* Create a block generator (i.e. a work scheduler) */
    BlockGenerator blockGenerator(outputSize, BLOCK_SIZE);

    int blockCount = blockGenerator.getBlockCount();

    #pragma omp parallel for
    for(int i=0; i<blockCount; i++) {
        /* Allocate memory for a small image block to be rendered by the current thread */
        ImageBlock block(Vector2i(BLOCK_SIZE,BLOCK_SIZE));

        /* Request an image block from the block generator */
        blockGenerator.next(block);

        /* Render all contained pixels */
        renderBlock(scene,block);

        /* The image block has been processed. Now add it to
               the "big" block that represents the entire image */
        result->put(block);
    }

    t = clock() - t;
    std::cout << "Raytracing time : " << float(t)/CLOCKS_PER_SEC << "s  -  nb triangle intersection: " << Mesh::ms_itersection_count << "\n";

    *done = true;
}

int main(int argc, char *argv[])
{
    getFileResolver()->prepend(DATA_DIR);

    try {
        nanogui::init();
        Viewer *screen = new Viewer();;

        if (argc == 2) {
            /* load file from the command line */
            filesystem::path path(argv[1]);

            if(path.extension() == "scn") { // load scene file
                screen->loadScene(argv[1]);
            }else if(path.extension() == "exr") { // load OpenEXR image
                screen->loadImage(argv[1]);
            }
        }

        /* Enter the application main loop */
        nanogui::mainloop();

        delete screen;
        nanogui::shutdown();
    } catch (const std::exception &e) {
        cerr << "Fatal error: " << e.what() << endl;
        return -1;
    }
    return 0;
}
