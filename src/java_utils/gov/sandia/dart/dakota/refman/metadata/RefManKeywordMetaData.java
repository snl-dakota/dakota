package gov.sandia.dart.dakota.refman.metadata;

public class RefManKeywordMetaData {

	public static final String BLURB = "Blurb";
	public static final String TOPICS = "Topics";
	public static final String SEE_ALSO = "See_Also";
	public static final String EXAMPLES = "Examples";
	public static final String DESCRIPTION = "Description";
	public static final String THEORY = "Theory";
	public static final String FAQ = "Faq";
	
	private String blurb = "";
	private String topics = "";
	private String seeAlso = "";
	private String examples = "";
	private String description = "";
	private String theory = "";
	private String faq = "";

	public String getBlurb() {
		return blurb;
	}

	public void setBlurb(String blurb) {
		this.blurb = blurb;
	}

	public String getTopics() {
		return topics;
	}

	public void setTopics(String topics) {
		this.topics = topics;
	}

	public String getSeeAlso() {
		return seeAlso;
	}

	public void setSeeAlso(String seeAlso) {
		this.seeAlso = seeAlso;
	}

	public String getExamples() {
		return examples;
	}

	public void setExamples(String examples) {
		this.examples = examples;
	}

	public String getDescription() {
		return description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	public String getTheory() {
		return theory;
	}

	public void setTheory(String theory) {
		this.theory = theory;
	}

	public String getFaq() {
		return faq;
	}

	public void setFaq(String faq) {
		this.faq = faq;
	}
	
	public void setByField(String field, String value) {
		if(field.equals(BLURB)) {
			setBlurb(value);
		} else if(field.equals(TOPICS)) {
			setTopics(value);
		} else if(field.equals(SEE_ALSO)) {
			setSeeAlso(value);
		} else if(field.equals(EXAMPLES)) {
			setExamples(value);
		} else if(field.equals(DESCRIPTION)) {
			setDescription(value);
		} else if(field.equals(THEORY)) {
			setTheory(value);
		} else if(field.equals(FAQ)) {
			setFaq(value);
		}
	}
	
	public boolean fieldAlreadySet(String field) {
		if(field.equals(BLURB)) {
			return !blurb.isBlank();
		} else if(field.equals(TOPICS)) {
			return !topics.isBlank();
		} else if(field.equals(SEE_ALSO)) {
			return !seeAlso.isBlank();
		} else if(field.equals(EXAMPLES)) {
			return !examples.isBlank();
		} else if(field.equals(DESCRIPTION)) {
			return !description.isBlank();
		} else if(field.equals(THEORY)) {
			return !theory.isBlank();
		} else if(field.equals(FAQ)) {
			return !faq.isBlank();
		}
		return false;
	}
}
