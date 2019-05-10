// To generate country names translation used by stk:
// in this directory,
// javac generate-country-names.java
// java CountryName > ../data/country_names.csv
// You may need to re-run this when new .po file is added to STK

import java.io.File;
import java.io.FilenameFilter;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;

class CountryName
{
    public static void main(String args[])
    {
        File po_folder = new File("../data/po");
        File[] po_list = po_folder.listFiles(new FilenameFilter()
            {
                public boolean accept(File dir, String name)
                {
                    return name.toLowerCase().endsWith(".po");
                }
            });
        TreeMap<String, Locale> po_locale_map = new TreeMap<String, Locale>();
        for (int i = 0; i < po_list.length; i++)
        {
            String po_file = po_list[i].getName();
            String po = po_file.substring(0, po_file.lastIndexOf(".")).replace('_', '-');
            po_locale_map.put(po, Locale.forLanguageTag(po));
        }

        System.out.print("country_code;");
        for (Map.Entry<String, Locale> entry : po_locale_map.entrySet())
        {
            System.out.print(entry.getKey());
            if (entry.getKey() != po_locale_map.lastKey())
                System.out.print(";");
        }
        System.out.print("\n");

        String[] country_codes = Locale.getISOCountries();
        for (String country_code : country_codes)
        {
            Locale country = new Locale("", country_code);
            System.out.print(country_code + ";");
            for (Map.Entry<String, Locale> entry : po_locale_map.entrySet())
            {
                System.out.print(country.getDisplayCountry(entry.getValue()));
                if (entry.getKey() != po_locale_map.lastKey())
                    System.out.print(";");
            }
            System.out.print("\n");
        }
    }
}
