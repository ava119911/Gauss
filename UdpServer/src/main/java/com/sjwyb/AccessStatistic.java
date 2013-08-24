package com.sjwyb;

import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class AccessStatistic extends AbstractLogStatistic {
	private Map<String, Long> ipCounter = new HashMap<String, Long>();
	
	private final SimpleDateFormat formatter = 
			new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	
	/*
	 * line example:
	 * 
	 * 2013-08-24 21:39:03<tab>125.78.168.7<tab>192.168.0.106:61808<tab>\
	 * http://search.jd.com/Search?keyword=%B0%FC%BD%AC<tab>\
	 * Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; SE 2.X MetaSr 1.0)
	 */

	@Override
	long parseTimeStamp(String line) {
		String[] parts = line.split("\\s+");
		Date date = null;
		try {
			date = formatter.parse(String.format("%s %s", parts[0], parts[1]));
		} catch (ParseException ex) {
			throw new RuntimeException("Parse timestamp from the following line failed:\n" + line + "\n");
		}
		
		return date.getTime();
	}

	@Override
	boolean incrementCounter(String line) {
		String[] parts = line.split("\\s+");
		String externalIp = parts[2];
		String internalIp = parts[3].split(":")[0];
		String ipPair = String.format("%s/%s", externalIp, internalIp);
		Long counter = ipCounter.get(ipPair);
		if (counter == null)
			counter = new Long(0);
		ipCounter.put(ipPair, new Long(counter + 1));
		return counter == 0;
	}
	
	public void outputResult() throws IOException {
		handleLogRecord();
		
		System.out.printf(
			"\nUnique IP Count\n"+
			"===============\n" +
			"       Today: %d\n" +
			"Last 3  days: %d\n" +
			"Last 7  days: %d\n" +
			"Last 15 days: %d\n" +
			"Last 30 days: %d\n",
			counters[TODAY],
			counters[LAST_3_DAYS],
			counters[LAST_7_DAYS], 
			counters[LAST_15_DAYS],
			counters[LAST_30_DAYS]);
		
		if (ipCounter.size() < 5)
			return;
		
		List<Map.Entry<String, Long>> list = new ArrayList<Map.Entry<String,Long>>(
				ipCounter.entrySet());
		Collections.sort(list, new Comparator<Map.Entry<String, Long>>() {
			public int compare(Map.Entry<String, Long> e1, Map.Entry<String, Long> e2) {
				long counter1 = e1.getValue();
				long counter2 = e2.getValue();
				return counter1 < counter2 ? 1 : (counter1 == counter2 ? 0 : -1);
			};
		});
		System.out.printf("\nTop 5 common ip\n");
		System.out.printf("===============\n");
		for (int i = 0; i < 5; i++) {
			System.out.printf("%s(%d)\n", list.get(i).getKey(), list.get(i).getValue());
		}
	}

	public static void main(String[] args) throws IOException {
		new AccessStatistic().outputResult();
	}

}
