//! [snippet1]
// We include what we need for the test
#include <gatb/gatb_core.hpp>

#include <iostream>
#include <memory>

// We use the required packages
using namespace std;

/********************************************************************************/
/*                  Iterate solid kmers from a HDF5 file (enhanced)             */
/*                                                                              */
/* This snippet shows how to iterate solid kmers from a file generated by DSK   */
/* or by dbgh5.                                                                 */
/*                                                                              */
/********************************************************************************/
int main (int argc, char* argv[])
{
    // We check that the user provides a graph URL (supposed to be in HDF5 format).
    if (argc < 3)
    {
        std::cerr << "You must provide the following." << std::endl;
        std::cerr << "  1) HDF5 file" << std::endl;
        std::cerr << "  2) 0 (no display)  1 (display kmers)   2 (display abundance distrib) or  " << std::endl;

        return EXIT_FAILURE;
    }

    bool display = atoi (argv[2]);

    // We get a handle on the HDF5 storage object.
    // Note that we use an auto pointer since the StorageFactory dynamically allocates an instance
    auto_ptr<Storage> storage (StorageFactory(STORAGE_HDF5).load (argv[1]));

    // We get the group for dsk
    Group& dskGroup = storage->getGroup("dsk");

    // We get the solid kmers collection 1) from the 'dsk' group  2) from the 'solid' collection
    Partition<Kmer<>::Count>& solidKmers = dskGroup.getPartition<Kmer<>::Count> ("solid", 32);

    // We can retrieve information (as an XML string) about the construction of the solid kmers
    cout << dskGroup.getProperty("xml") << endl;

    // We can access each of these information through a Properties object
    Properties props;
    props.readXML (dskGroup.getProperty("xml"));

    // Now, we can for instance get the kmer size (as an integer)
    cout << "kmer size:      " << props.getInt ("kmer_size")      << endl;
    cout << "nb solid kmers: " << props.getInt ("kmers_nb_solid") << endl;

    // We create a Model instance. It will help to dump the kmers in
    // a human readable form (ie as a string of nucleotides)
    Kmer<>::ModelCanonical model (props.getInt ("kmer_size"));

    size_t nbKmers = 0;

    // We create an iterator for our [kmer,abundance] values
    ProgressIterator<Kmer<>::Count> iter (solidKmers);

    Kmer<>::Type checksum;
    map<u_int64_t,u_int64_t> distrib;

    // We iterate the solid kmers from the retrieved collection
    for (iter.first(); !iter.isDone(); iter.next())
    {
        // shortcut
        Kmer<>::Count& count = iter.item();

        // We update the checksum.
        checksum += count.value;

        // We update the distribution
        distrib [count.abundance] ++;

        // We dump the solid kmer information:
        //   1) nucleotides
        //   2) raw value (integer)
        //   3) abundance
        if (display==1)
        {
            cout << "[" << ++nbKmers << "]  " << model.toString(count.value) << "  " << count.value << "  "  << count.abundance << endl;
        }
    }

    cout << "kmer checksum:  " << checksum << endl;

    if (display==2)
    {
        for (map<u_int64_t,u_int64_t>::iterator it = distrib.begin(); it != distrib.end(); ++it)
        {
            cout << it->first << "  "  << it->second << endl;
        }
    }
}
//! [snippet1]
