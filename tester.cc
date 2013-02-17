#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

int main()
{
    using namespace boost::gregorian;
    using namespace boost::posix_time;
    using namespace std;

    stringstream ss;

    /****** format strings ******/
    time_facet* output_facet = new time_facet();
    time_input_facet* input_facet = new time_input_facet();
    ss.imbue(locale(ss.getloc(), output_facet));
    ss.imbue(locale(ss.getloc(), input_facet));
    output_facet->format(time_facet::iso_time_format_extended_specifier);
    input_facet->format(time_input_facet::iso_time_format_extended_specifier);
    ptime t(second_clock::local_time());
    ss << t;
    cout << ss.str() << endl; // 2012-08-03 23:46:38
    ss.str("2000-01-31 12:34:56");
    ss >> t;
    assert(t.date().year() == 2000);
    assert(t.date().month() == 1);
    assert(t.date().day() == 31);
    assert(t.time_of_day().hours() == 12);
    assert(t.time_of_day().minutes() == 34);
    assert(t.time_of_day().seconds() == 56);
}