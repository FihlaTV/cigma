#include <iostream>
#include <cassert>

#include "EvalCmd.h"
#include "StringUtils.h"

using namespace std;
using namespace cigma;

// ---------------------------------------------------------------------------

EvalCmd::EvalCmd()
{
    name = "eval";

    field = 0;
    points = 0;
    values = 0;
}

EvalCmd::~EvalCmd()
{
    // XXX:
}


// ---------------------------------------------------------------------------


void EvalCmd::setupOptions(AnyOption *opt)
{
    assert(opt != 0);

    /* setup usage */
    opt->addUsage("Usage:");
    opt->addUsage("   cigma eval [options]");
    opt->addUsage("     --field     Input field");
    opt->addUsage("     --points    Input points");
    opt->addUsage("     --output    Output values");

    /* setup flags and options */
    opt->setFlag("help",'h');
    opt->setFlag("verbose",'v');

    opt->setOption("field"); // XXX: -f short option?
    opt->setOption("field-mesh");
    opt->setOption("field-mesh-coordinates");
    opt->setOption("field-mesh-connectivity");
    opt->setOption("field-function");

    opt->setOption("points"); // XXX: -p short option?
    opt->setOption("output");
}

void EvalCmd::configure(AnyOption *opt)
{
    assert(opt != 0);

    string inputstr;
    char *in;


    // read verbose flag
    verbose = opt->getFlag("verbose");


    /* determine the output values path */
    in = opt->getValue("output");
    if (in == 0)
    {
        cerr << "eval: Please specify an output path for "
             << "the values dataset (missing --output option)"
             << endl;
        exit(1);
    }
    inputstr = in;

    string valuesExt;
    parse_dataset_path(inputstr, valuesLoc, valuesFile, valuesExt);
    valuesWriter = NewWriter(valuesExt.c_str());
    if (valuesWriter->getType() == Writer::NULL_WRITER)
    {
        cerr << "eval: Could not make writer for output file. "
             << "Unsupported extension (" << valuesExt << ")."
             << endl;
        exit(1);
    }
    if ((valuesLoc == "") && (valuesWriter->getType() == Writer::HDF_WRITER))
    {
        valuesLoc = "/values";
    }


    /* determine the input points path */
    in = opt->getValue("points");
    if (in == 0)
    {
        cerr << "eval: Please specify an input path for "
             << "the points dataset (missing --points option)"
             << endl;
        exit(1);
    }

    pointsReader.load_args(opt, "points");
    pointsReader.validate_args("eval");
    pointsReader.load_points();
    if (pointsReader.points == 0)
    {
        cerr << "eval: Could not load points dataset!" << endl;
        exit(1);
    }

    /* determine the input field */
    fieldReader.load_args(opt, "field");
    fieldReader.validate_args("eval");
    fieldReader.load_field();
    if (fieldReader.field == 0)
    {
        cerr << "eval: Could not load field from path "
             << fieldReader.fieldPath
             << endl;
        exit(1);
    }


    if (field->n_dim() != points->n_dim())
    {
        cerr << "eval: Domain dimensions do not match: "
             << "field->dim = " << field->n_dim() << ", "
             << "points->dim = " << points->n_dim()
             << endl;
        exit(1);
    }

}

int EvalCmd::run()
{
    assert(field != 0);
    assert(points != 0);
    assert(values != 0);

    // indices
    int i;
    int e;

    // dimensions
    const int npts = points->n_points();
    const int valdim = field->n_rank();

    // data
    double *phi = new double[npts * valdim];

    for (i = 0; i < npts; i++)
    {
        double *globalPoint = (*points)[i];
        field->eval(globalPoint, &phi[valdim*i]);
    }

    
    //
    // output values dataset
    //

    int ierr;
    
    cout << "Creating file " << valuesFile << endl;

    ierr = valuesWriter->open(valuesFile.c_str());
    if (ierr < 0)
    {
        cerr << "eval: Could not open (or create) output file " << valuesFile << endl;
        exit(1);
    }

    ierr = valuesWriter->write_dataset(valuesLoc.c_str(), phi, npts, valdim);
    if (ierr < 0)
    {
        cerr << "eval: Could not create values dataset " << valuesLoc << endl;
        exit(1);
    }

    ierr = valuesWriter->close();
    if (ierr < 0)
    {
        cerr << "eval: Error while closing output file?" << endl;
    }

    //
    // clean up
    //

    delete [] phi; // XXX: auto_ptr?

    return 0;
}

// ---------------------------------------------------------------------------
