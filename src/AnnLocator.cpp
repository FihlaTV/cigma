#include <cassert>
#include "AnnLocator.h"


// ---------------------------------------------------------------------------

cigma::AnnLocator::AnnLocator()
{
    nnk = 8;
    epsilon = 0;

    npts = 0;
    ndim2 = 0;

    dataPoints = 0;
    kdtree = 0;

    nnIdx = 0;
    nnDists = 0;

    locatorType = NULL_LOCATOR;
}

cigma::AnnLocator::~AnnLocator()
{
    if (kdtree != 0) delete kdtree;

    if (dataPoints != 0)
    {
        if (locatorType == CELL_LOCATOR)
        {
            annDeallocPts(dataPoints);
        }
    }

    if (nnIdx != 0) delete [] nnIdx;
    if (nnDists != 0) delete [] nnDists;
}


// ---------------------------------------------------------------------------

void cigma::AnnLocator::initialize(MeshPart *meshPart)
{
    assert(nnk > 0);

    npts = meshPart->nel;
    ndim = meshPart->nsd;
    ndim2 = ndim * 2;
    
    assert(npts > 0);
    assert(ndim > 0);

    dataPoints = annAllocPts(npts, ndim);
    queryPoint = annAllocPt(ndim2);

    nnIdx = new ANNidx[nnk];
    nnDists = new ANNdist[nnk];

    int i,j;
    double minpt[ndim];
    double maxpt[ndim];

    for (i = 0; i < npts; i++)
    {
        ANNpoint pt = dataPoints[i];
        meshPart->select_cell(i);
        meshPart->cell->bbox(minpt, maxpt);
        for (j = 0; j < ndim; j++)
        {
            pt[ndim*0 + j] = minpt[j];
            pt[ndim*1 + j] = maxpt[j];
        }
    }

    kdtree = new ANNkd_tree(dataPoints, npts, ndim2);

    locatorType = CELL_LOCATOR;
}


// ---------------------------------------------------------------------------

void cigma::AnnLocator::initialize(Points *points)
{
    assert(nnk > 0);

    npts = points->n_points();
    ndim = points->n_dim();

    assert(npts > 0);
    assert(ndim > 0);

    // XXX watch out for when you change the ANNpoint type to float
    assert(sizeof(ANNpoint) == sizeof(double));

    dataPoints = (ANNpointArray)(points->data);
    queryPoint = annAllocPt(ndim);

    nnIdx = new ANNidx[nnk];
    nnDists = new ANNdist[nnk];

    kdtree = new ANNkd_tree(dataPoints, npts, ndim);

    locatorType = POINT_LOCATOR;
}


// ---------------------------------------------------------------------------

void cigma::AnnLocator::search(double *point)
{
    for (int i = 0; i < ndim; i++)
    {
        queryPoint[ndim*0 + i] = point[i];
        queryPoint[ndim*1 + i] = point[i];
    }

    kdtree->annkSearch(queryPoint, nnk, nnIdx, nnDists, epsilon);
}


// ---------------------------------------------------------------------------
