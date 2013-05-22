#ifndef L1SALIENTMODEL_H
#define L1SALIENTMODEL_H

#include "video.h"
#include "l1model.h"

class L1SalientModel : public L1Model
{
public:
    int salientSlackVarPerFrame;


    L1SalientModel(int frameCount);

    // This is the entry function
    bool prepare(Video* video);

    // Done
    void setObjectives();

    // Done
    int toSalientSlackIndex(int t, int corner, char component);

    // Done
    void setInclusionConstraints(Rect cropbox, int videoWidth, int videoHeight);

    // Done
    void setSalientConstraints(Video* video);

};

#endif // L1SALIENTMODEL_H
