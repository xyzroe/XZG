/**
 * Toast (https://github.com/itchief/ui-components/tree/master/toast)
 * Copyright 2020 - 2021 Alexander Maltsev
 * Licensed under MIT (https://github.com/itchief/ui-components/blob/master/LICENSE)
**/

class Toast {
  constructor(params) {
    this._title = params['title'] === false ? false : params['title'] || 'Title';
    this._text = params['text'] || 'Message...';
    this._theme = params['theme'] || 'default';
    this._autohide = params['autohide'] && true;
    this._interval = +params['interval'] || 5000;
    this._create();
    this._el.addEventListener('click', (e) => {
      if (e.target.classList.contains('toast__close')) {
        this._hide();
      }
    });
    this._show();
  }
  _show() {
    this._el.classList.add('toast_showing');
    this._el.classList.add('toast_show');
    window.setTimeout(() => {
      this._el.classList.remove('toast_showing');
    });
    if (this._autohide) {
      setTimeout(() => {
        this._hide();
      }, this._interval);
    }
  }
  _hide() {
    this._el.classList.add('toast_showing');
    this._el.addEventListener('transitionend', () => {
      this._el.classList.remove('toast_showing');
      this._el.classList.remove('toast_show');
      this._el.remove();
    }, {once : true});
    const event = new CustomEvent('hide.toast', { detail: { target: this._el } });
    document.dispatchEvent(event);
  }
  _create() {
    const el = document.createElement('div');
    el.classList.add('toast');
    el.classList.add(`toast_${this._theme}`);
    let html = `{header}<div class="toast__body"></div><button class="toast__close" type="button"></button>`;
    const htmlHeader = this._title === false ? '' : '<div class="toast__header"></div>';
    html = html.replace('{header}', htmlHeader);
    el.innerHTML = html;
    if (this._title) {
      el.querySelector('.toast__header').textContent = this._title;
    } else {
      el.classList.add('toast_message');
    }
    el.querySelector('.toast__body').textContent = this._text;
    this._el = el;
    if (!document.querySelector('.toast-container')) {
      const container = document.createElement('div');
      container.classList.add('toast-container');
      document.body.append(container);
    }
    document.querySelector('.toast-container').append(this._el);
  }
}


function checkLatestRelease() {
  document.addEventListener('hide.toast', (e) => {
    console.log(e.detail.target);
  });
	$.getJSON("https://api.github.com/repos/smlight-dev/SLZB-06/releases/latest").done(function(release) {
	  var asset = release.assets[0];
	  var downloadCount = 0;
	  for (var i = 0; i < release.assets.length; i++) {
		downloadCount += release.assets[i].download_count;
	  }

	  var releaseInfo = "New firmware version (" + release.name + ") was found on GitHub. It was downloaded " + downloadCount.toLocaleString() + " times. Go -> Tools -> Update ESP32 page to get more info.";

    var div1 = document.getElementById('ver');

    const exampleAttr= div1.getAttribute('v');

    if (exampleAttr != release.name) {
      new Toast({
        title: false,
        text: releaseInfo,
        theme: "warning",
        autohide: false,
        interval: false
      });
    }

	});
  }
