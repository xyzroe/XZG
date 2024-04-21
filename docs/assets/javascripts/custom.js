document.addEventListener("DOMContentLoaded", function () {
    var titleElement = document.querySelector('.md-header__title');
    if (titleElement) {
        var customBlock = document.createElement('div');
        customBlock.innerHTML = `
            <div class="language-selector">
                <div class="flag-icon">🌍</div>
                <div class="language-list" style="display: none;">
                    <a href="#" alt="EN" data-google-lang="en" class="language__img">🇬🇧</a>
                    <a href="#" alt="UA" data-google-lang="uk" class="language__img">🇺🇦</a>
                    <a href="#" alt="PL" data-google-lang="pl" class="language__img">🇵🇱</a>
                    <a href="#" alt="IL" data-google-lang="iw" class="language__img">🇮🇱</a>
                    <a href="#" alt="DE" data-google-lang="de" class="language__img">🇩🇪</a>
                    <a href="#" alt="FR" data-google-lang="fr" class="language__img">🇫🇷</a>
                    <a href="#" alt="ES" data-google-lang="es" class="language__img">🇪🇸</a>
                    <a href="#" alt="IT" data-google-lang="it" class="language__img">🇮🇹</a>
                    <a href="#" alt="NL" data-google-lang="nl" class="language__img">🇳🇱</a>
                    <a href="#" alt="RU" data-google-lang="ru" class="language__img">🇷🇺</a>
                    <a href="#" alt="ZH" data-google-lang="zh-CN" class="language__img">🇨🇳</a>
                </div>
            </div>`;
        customBlock.className = 'language';
        titleElement.parentNode.insertBefore(customBlock, titleElement.nextSibling);
    }
});


document.addEventListener('DOMContentLoaded', function () {
    var flagIcon = document.querySelector('.flag-icon');
    var languageList = document.querySelector('.language-list');

    flagIcon.addEventListener('click', function () {
        languageList.style.display = languageList.style.display === 'none' ? 'flex' : 'none';
    });

    document.addEventListener('click', function (event) {
        if (!flagIcon.contains(event.target) && !languageList.contains(event.target)) {
            languageList.style.display = 'none';
        }
    });
});


