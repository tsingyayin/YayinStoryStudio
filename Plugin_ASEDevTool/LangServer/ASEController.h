#pragma once

class QString;
class AbstractASEController {
public:
	AbstractASEController();
	void setControllerName(const QString& name);
	void setControllerASRule(const QString& rule);
};