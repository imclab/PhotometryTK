// __BEGIN_LICENSE__
// Copyright (C) 2006-2011 United States Government as represented by
// the Administrator of the National Aeronautics and Space Administration.
// All Rights Reserved.
// __END_LICENSE__


// What's this file supposed to do ?
//
// (Pho)tometry (Init)ialize Phofile
//
// It's used to create project file and to solve for the starting ET.
//
// Solving for ET goes a like this. Ti =To*Ro/Ri. Where the R equals
// the average reflectance. To is the starting exposure time .. it
// needs to be provided by the user .. but I'm just going to provide 1.
//
// If we're not calculating R, all ET get defined as 1.

#include <vw/Image.h>
#include <photk/ProjectFileIO.h>
#include <photk/Macros.h>
#include <photk/Common.h>
using namespace vw;
using namespace photk;

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace std;

struct Options : photk::BaseOptions {
  // Input for project file
  int32 max_iterations;
  std::string reflectance_type, datum, channel_type;
  double rel_tol, abs_tol;

  // Output
  std::string output_mode;
  std::string output_prefix;
};

void create_ptk( Options const& opt ) {
  ProjectMeta proj_meta;
  proj_meta.set_name( opt.output_prefix );
  proj_meta.set_num_cameras( 0 ); // Phodrg2plate .. will add the cameras
  proj_meta.set_max_iterations( opt.max_iterations );
  proj_meta.set_rel_tol( opt.rel_tol );
  proj_meta.set_abs_tol( opt.abs_tol );
  proj_meta.set_min_pixval( 0 );
  proj_meta.set_max_pixval( 0 );

  std::string reftype = opt.reflectance_type;
  boost::to_lower( reftype );
  if ( reftype == "lamb" || reftype == "lambertian" ) {
    proj_meta.set_reflectance( ProjectMeta::LAMBERTIAN );
  } else if ( reftype == "gaskall" || reftype == "lunarl_gaskall" ) {
    proj_meta.set_reflectance( ProjectMeta::LUNARL_GASKALL );
  } else if ( reftype == "mcewen" || reftype == "lunarl_mcewen" ) {
    proj_meta.set_reflectance( ProjectMeta::LUNARL_MCEWEN );
  } else {
    proj_meta.set_reflectance( ProjectMeta::NONE );
  }

  proj_meta.set_plate_manager( opt.output_mode );
  proj_meta.set_datum_name( opt.datum );
  proj_meta.set_drg_channel_type( opt.channel_type );

  std::list<CameraMeta> emptycams;
  write_pho_project( opt.output_prefix, proj_meta,
                     emptycams.begin(), emptycams.end() );
}

void handle_arguments( int argc, char *argv[], Options& opt ) {
  po::options_description general_options("");
  general_options.add_options()
    ("max_iterations", po::value(&opt.max_iterations)->default_value(100), "")
    ("mode,m", po::value(&opt.output_mode)->default_value("equi"),
     "Output mode [toast, equi, polar]")
    ("channel_type", po::value(&opt.channel_type)->default_value("FLOAT32"),
     "DRG channel type [FLOAT32, INT16, UINT8]")
    ("datum,d", po::value(&opt.datum)->default_value("D_MOON"),
     "Datum options are [WGS84,WGS72,D_MOON,D_MARS]")
    ("reflectance_type", po::value(&opt.reflectance_type)->default_value("none"), "Reflectance options are [none, lambertian, gaskall, mcewen]")
    ("rel_tol", po::value(&opt.rel_tol)->default_value(1e-2),
     "Minimium required amount of improvement between iterations")
    ("abs_tol", po::value(&opt.abs_tol)->default_value(1e-2),
     "Error shutoff threshold.");
  general_options.add( photk::BaseOptionsDescription(opt) );

  po::options_description positional("");
  positional.add_options()
    ("output_prefix", po::value(&opt.output_prefix), "Output prefix");

  po::positional_options_description positional_desc;
  positional_desc.add("output_prefix", 1);

  std::ostringstream usage;
  usage << "Usage: " << argv[0] << " <projectname> <optional project settings> \n";

  po::variables_map vm =
    photk::check_command_line( argc, argv, opt, general_options,
                             positional, positional_desc, usage.str() );

  boost::to_lower( opt.output_mode );
  if ( !( opt.output_mode == "equi" || opt.output_mode == "toast" ||
          opt.output_mode == "polar" ) )
    vw_throw( ArgumentErr() << "Unknown mode: \"" << opt.output_mode
              << "\".\n\n" << usage.str() << general_options );
  if ( opt.output_prefix.empty() )
    vw_throw( ArgumentErr() << "Missing output prefix!\n"
              << usage.str() << general_options );

  boost::to_lower( opt.channel_type );
  if ( !( opt.channel_type == "float32" || opt.channel_type == "int16" ||
          opt.channel_type == "uint8" ) )
    vw_throw( ArgumentErr() << "Unknown mode: \"" << opt.channel_type
              << "\".\n\n" << usage.str() << general_options );
}

int main( int argc, char *argv[] ) {

  Options opt;
  try {
    handle_arguments( argc, argv, opt );
    create_ptk( opt );
  } PHOTK_STANDARD_CATCHES;

  return 0;
}
